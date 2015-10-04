//
// Tomato Media Gaming
// 设备管理器
// 
// 作者：SunnyCase 
// 创建日期 2015-09-04
#include "pch.h"
#include "DeviceManager.h"
#include <dxgi1_2.h>

using namespace NS_MEDIA;
using namespace NS_MEDIA_GAMING;
using namespace WRL;

#if (NTDDI_VERSION >= NTDDI_WIN8)
DeviceManager::DeviceManager(ISwapChainPanelNative* swapChainPanel, uint32_t width, uint32_t height)
{
	CreateDeviceResources();
	CreateCompositionSwapChain(width, height);
	AttachSwapChainPanel(swapChainPanel);
}
#endif

TextureLoader & DeviceManager::GetTextureLoader() const
{
	if (!_textureLoader)
		ThrowIfFailed(E_NOT_VALID_STATE);
	return *_textureLoader;
}

void DeviceManager::CreateDeviceResources()
{
	// 创建 D3D 设备
	UINT flags = 0;
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
	
	// 获取 DXGIFactory
	ComPtr<IDXGIDevice2> dxgiDevice;
	ThrowIfFailed(d3dDevice.As(&dxgiDevice));
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));
	LoggingDeviceInfo();

	ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
	// 创建其他资源
	_textureLoader = std::make_unique<TextureLoader>(d3dDevice.Get());
}

#if (NTDDI_VERSION >= NTDDI_WIN8)
void DeviceManager::CreateCompositionSwapChain(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0) ThrowIfFailed(E_INVALIDARG);

	DXGI_SWAP_CHAIN_DESC1 desc{ 0 };
	desc.Width = width;
	desc.Height = height;
	desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	desc.BufferCount = 2;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	ThrowIfFailed(dxgiFactory->CreateSwapChainForComposition(d3dDevice.Get(), &desc, nullptr, &dxgiSwapChain));
}

void DeviceManager::AttachSwapChainPanel(ISwapChainPanelNative * swapChainPanel)
{
	if (swapChainPanel)
		ThrowIfFailed(swapChainPanel->SetSwapChain(dxgiSwapChain.Get()));
}
#endif

void DeviceManager::LoggingDeviceInfo()
{
	ComPtr<IDXGIDevice2> dxgiDevice;
	ThrowIfFailed(d3dDevice.As(&dxgiDevice));
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

	DXGI_ADAPTER_DESC desc;
	ThrowIfFailed(dxgiAdapter->GetDesc(&desc));

	std::wstringstream ss;
	ss << std::endl << L"Adapter Information: ";
	ss << std::endl << L"Device Id: " << desc.DeviceId;
	ss << std::endl << L"Description: " << desc.Description;
	ss << std::endl << L"Dedicated Video Memory: " << desc.DedicatedVideoMemory << L" Bytes";
	ss << std::endl << L"Shared System Memory: " << desc.SharedSystemMemory << L" Bytes";
	auto message = ss.str();
	
#ifdef __cplusplus_winrt
	auto logger = ref new Core::Logger(DeviceManager::typeid->FullName);
	logger->Information(ref new Platform::String(message.c_str(), message.length()));
#endif
}
