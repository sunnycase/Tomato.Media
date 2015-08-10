//
// Tomato Media
// D3D11 视频渲染器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-10
#include "pch.h"
#include "D3D11VideoRender.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

D3D11VideoRender::D3D11VideoRender()
{
}

void D3D11VideoRender::Initialize()
{
	CreateDeviceResources();
}

void D3D11VideoRender::CreateDeviceResources()
{
	// 创建 D3D 设备
	D3D11_CREATE_DEVICE_FLAG flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		d3dDevice.ReleaseAndGetAddressOf(), &featureLevel, d3dDeviceContext.ReleaseAndGetAddressOf()));
}

ComPtr<ID3D11Texture2D> D3D11VideoRender::CreateFrame(IMFSample * sample, UINT width, UINT height)
{
	// 获取 Buffer 数量
	DWORD bufferCount;
	ThrowIfFailed(sample->GetBufferCount(&bufferCount));

	ComPtr<ID3D11Texture2D> texture;
	for (DWORD i = 0; i < 1; i++)
	{
		ComPtr<IMFMediaBuffer> buffer;
		ThrowIfFailed(sample->GetBufferByIndex(i, &buffer));

		ComPtr<IMFDXGIBuffer> dxgiBuffer;
		if (SUCCEEDED(buffer.As(&dxgiBuffer)))
			ThrowIfFailed(dxgiBuffer->GetResource(IID_PPV_ARGS(&texture)));
		else
		{
			ComPtr<IMF2DBuffer> buffer2d;
			ThrowIfFailed(buffer.As(&buffer2d));

			D3D11_TEXTURE2D_DESC desc{ 0 };
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_NV12;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			BYTE* base; LONG pitch;
			ThrowIfFailed(buffer2d->Lock2D(&base, &pitch));
			finalizer fin([&] { buffer2d->Unlock2D();});

			D3D11_SUBRESOURCE_DATA data{ base, static_cast<UINT>(pitch), 0 };

			ThrowIfFailed(d3dDevice->CreateTexture2D(&desc, &data, &texture));
		}
	}
	return texture;
}

void D3D11VideoRender::RenderFrame(ID3D11Texture2D * texture)
{
	RenderFrameToSurfaceImageSource(texture);
}

#ifdef __cplusplus_winrt

void D3D11VideoRender::SetSurfaceImageSource(Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ imageSource, UINT width, UINT height)
{
	sisDispatcher = nullptr;
	sisNative.Reset();

	if (imageSource)
	{
		sisDispatcher = imageSource->Dispatcher;
		ComPtr<IUnknown> unkImageSourceNative(reinterpret_cast<IUnknown*>(imageSource));
		ThrowIfFailed(unkImageSourceNative.As(&sisNative));

		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(d3dDevice.As(&dxgiDevice));
		ThrowIfFailed(sisNative->SetDevice(dxgiDevice.Get()));
		sisWidth = width;
		sisHeight = height;
	}
}

#endif

#include <windows.ui.core.h>

void D3D11VideoRender::RenderFrameToSurfaceImageSource(ID3D11Texture2D* texture)
{
#ifdef __cplusplus_winrt
	sisDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]
	{
		ComPtr<IDXGISurface> surface;
		POINT offset;

		auto beginDrawHR = sisNative->BeginDraw({ 0, 0, static_cast<LONG>(sisWidth), static_cast<LONG>(sisHeight) }, &surface, &offset);
		if (beginDrawHR == DXGI_ERROR_DEVICE_REMOVED || beginDrawHR == DXGI_ERROR_DEVICE_RESET)
		{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
			::RoOriginateError(beginDrawHR, nullptr);
#endif
		}
		else
		{
			finalizer fin([&] {sisNative->EndDraw();});

			ComPtr<ID3D11Texture2D> renderTarget;
			ThrowIfFailed(surface.As(&renderTarget));

			ComPtr<ID3D11RenderTargetView> renderTargetView;
			ThrowIfFailed(d3dDevice->CreateRenderTargetView(renderTarget.Get(), nullptr, &renderTargetView));

			static const float clearColor[] = { 1.f, 1.f, 0.f, 1.f };
			d3dDeviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);
		}
	}));
#endif
}