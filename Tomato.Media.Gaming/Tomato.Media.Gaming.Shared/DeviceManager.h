//
// Tomato Media Gaming
// 设备管理器
// 
// 作者：SunnyCase 
// 创建日期 2015-09-04
#pragma once
#include "common.h"
#include <d3d11.h>
#include <dxgi.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <DirectXMath.h>

DEFINE_NS_MEDIA_GAMING

///<summary>设备管理器</summary>
class DeviceManager
{
public:
#if (NTDDI_VERSION >= NTDDI_WIN8)
	DeviceManager(ISwapChainPanelNative* swapChainPanel, uint32_t width, uint32_t height);
#endif
private:
	void CreateDeviceResources();
	void CreateCompositionSwapChain(uint32_t width, uint32_t height);

#if (NTDDI_VERSION >= NTDDI_WIN8)
	void AttachSwapChainPanel(ISwapChainPanelNative* swapChainPanel);
#endif

	void LoggingDeviceInfo();
private:
	WRL::ComPtr<ID3D11Device> d3dDevice;
	WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain;
	D3D_FEATURE_LEVEL featureLevel;
};

END_NS_MEDIA_GAMING