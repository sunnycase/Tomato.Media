//
// Tomato Media
// D3D11 视频渲染器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-10
#include "pch.h"
#include "D3D11VideoRender.h"
#include "ResourceHelper.h"
#include <DirectXMath.h>
#include "Utility/WICHelper.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;
using namespace DirectX;

namespace
{
	struct alignas(16) Vertex
	{
		XMFLOAT4A Position;
		XMFLOAT4A Color;
		XMFLOAT2A TexCoord;
	};

	enum { e = sizeof(Vertex) };
	static_assert(sizeof(Vertex) == 48, "sizeof Vertex must be 12.");

	static WICHelper& GetWIC()
	{
		static WICHelper wic;
		return wic;
	}
}

D3D11VideoRender::D3D11VideoRender()
{
}

task<void> D3D11VideoRender::Initialize()
{
	if (!initStarted.exchange(true))
		initTask = InitializeCore();
	return initTask;
}

task<void> D3D11VideoRender::InitializeCore()
{
	return CreateDeviceResources();
}

task<void> D3D11VideoRender::CreateDeviceResources()
{
	// 创建 D3D 设备
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
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

	// 加载并设置 Vertex Shader
	ComPtr<D3D11VideoRender> thisGuard(this);
	return BlobResource::LoadFromResource(Resources::DefaultVideoVS)
		.then([thisGuard, this](std::shared_ptr<BlobResource> blob)
	{
		ComPtr<ID3D11VertexShader> vertexShader;
		ThrowIfFailed(d3dDevice->CreateVertexShader(blob->GetPointer(), blob->GetLength(), nullptr, &vertexShader));
		d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);

		// 加载并设置 Input Layout
		ComPtr<ID3D11InputLayout> inputLayout;
		D3D11_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		ThrowIfFailed(d3dDevice->CreateInputLayout(inputElements, ARRAYSIZE(inputElements),
			blob->GetPointer(), blob->GetLength(), &inputLayout));
		d3dDeviceContext->IASetInputLayout(inputLayout.Get());

		// 加载 Pixel Shader
		return BlobResource::LoadFromResource(Resources::DefaultVideoPS);
	}).then([thisGuard, this](std::shared_ptr<BlobResource> blob)
	{
		// 设置 Pixel Shader
		ComPtr<ID3D11PixelShader> pixelShader;
		ThrowIfFailed(d3dDevice->CreatePixelShader(blob->GetPointer(), blob->GetLength(), nullptr, &pixelShader));
		d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

		CreateVideoLayerResources();
		// 设置设备状态
		ConfigureDeviceState();
	});
}

void D3D11VideoRender::ConfigureDeviceState()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState;
	ThrowIfFailed(d3dDevice->CreateSamplerState(&desc, &samplerState));
	d3dDeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
}

void D3D11VideoRender::CreateVideoLayerResources()
{
	// Vertex Buffer
	Vertex vertices[] = {
		{ { +1.f, +1.f, 0.9999999f, 1.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f } },
		{ { +1.f, -1.f, 0.9999999f, 1.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 1.f } },
		{ { -1.f, +1.f, 0.9999999f, 1.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 0.f } },
		{ { -1.f, -1.f, 0.9999999f, 1.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f } }
	};
	D3D11_BUFFER_DESC desc = { 0 };
	desc.ByteWidth = sizeof(vertices);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = vertices;
	data.SysMemPitch = 0;
	ThrowIfFailed(d3dDevice->CreateBuffer(&desc, &data, &videoLayerVB));
}

Frame D3D11VideoRender::CreateFrame(IMFSample * sample, UINT width, UINT height)
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
		if (!SUCCEEDED(buffer.As(&dxgiBuffer)))
		{
			ThrowIfFailed(dxgiBuffer->GetResource(IID_PPV_ARGS(&texture)));
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);
			desc.Height = desc.Height;
		}
		else
		{
			ComPtr<IMF2DBuffer> buffer2d;
			ThrowIfFailed(buffer.As(&buffer2d));

			DWORD bufferSize;
			ThrowIfFailed(buffer2d->GetContiguousLength(&bufferSize));

			BYTE* base; LONG pitch;
			ThrowIfFailed(buffer2d->Lock2D(&base, &pitch));
			auto fin = make_finalizer([&] { buffer2d->Unlock2D();});

			//width = pitch;
			height = bufferSize / pitch * 2 / 3;

			D3D11_TEXTURE2D_DESC desc{ 0 };
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_NV12;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA data{ base, static_cast<UINT>(pitch), 0 };
			ThrowIfFailed(d3dDevice->CreateTexture2D(&desc, &data, &texture));
		}
	}
	ComPtr<ID3D11ShaderResourceView> luminanceView, chrominanceView;
	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(
		texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8_UNORM);
	ThrowIfFailed(d3dDevice->CreateShaderResourceView(texture.Get(), &resDesc, &luminanceView));

	resDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(
		texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8_UNORM);
	ThrowIfFailed(d3dDevice->CreateShaderResourceView(texture.Get(), &resDesc, &chrominanceView));
	return{ luminanceView, chrominanceView };
}

void D3D11VideoRender::RenderFrame(const Frame& frame)
{
	RenderFrameToSurfaceImageSource(frame);
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

		Initialize().then([=] {
			// Viewport
			D3D11_VIEWPORT viewport = { 0 };
			viewport.Width = static_cast<FLOAT>(width);
			viewport.Height = static_cast<FLOAT>(height);
			viewport.MinDepth = 0.f;
			viewport.MaxDepth = 1.f;
			d3dDeviceContext->RSSetViewports(1, &viewport);
		});
	}
}

#endif

void D3D11VideoRender::RenderFrameToSurfaceImageSource(const Frame& frame)
{
	if (!initTask.is_done())
		ThrowIfFailed(E_NOT_VALID_STATE);

#ifdef __cplusplus_winrt
	Frame frameHolder(frame);
	sisDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]
	{
		ComPtr<IDXGISurface> surface;
		POINT offset;

		auto beginDrawHR = sisNative->BeginDraw({ 0, 0, static_cast<LONG>(sisWidth), static_cast<LONG>(sisHeight) }, &surface, &offset);
		if (beginDrawHR == DXGI_ERROR_DEVICE_REMOVED || beginDrawHR == DXGI_ERROR_DEVICE_RESET ||
			FAILED(beginDrawHR))
		{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
			::RoOriginateError(beginDrawHR, nullptr);
#endif
		}
		else
		{
			auto fin = make_finalizer([&] {sisNative->EndDraw();});

			ComPtr<ID3D11Texture2D> renderTarget;
			ThrowIfFailed(surface.As(&renderTarget));

			ComPtr<ID3D11RenderTargetView> renderTargetView;
			ThrowIfFailed(d3dDevice->CreateRenderTargetView(renderTarget.Get(), nullptr, &renderTargetView));

			DrawVideoLayer(renderTargetView.GetAddressOf(), frameHolder);
		}
	}));
#endif
}

void D3D11VideoRender::DrawVideoLayer(ID3D11RenderTargetView** target, const Frame& frame)
{
	static const float clearColor[] = { 1.f, 1.f, 1.f, 1.f };
	d3dDeviceContext->ClearRenderTargetView(*target, clearColor);

	d3dDeviceContext->OMSetRenderTargets(1, target, nullptr);

	UINT strides = sizeof(Vertex); UINT offset = 0;
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3dDeviceContext->IASetVertexBuffers(0, 1, videoLayerVB.GetAddressOf(), &strides, &offset);

	ID3D11ShaderResourceView* resViews[] = { frame.Luminance.Get(), frame.Chrominance.Get() };
	d3dDeviceContext->PSSetShaderResources(0, 2, resViews);
	d3dDeviceContext->Draw(4, 0);
}