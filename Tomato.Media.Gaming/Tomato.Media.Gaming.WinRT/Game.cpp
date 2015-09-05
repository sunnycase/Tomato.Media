//
// Tomato Media Gaming
// 游戏主体
// 
// 作者：SunnyCase 
// 创建日期 2015-09-05
#include "pch.h"
#include "Game.h"
#include <windows.ui.xaml.media.dxinterop.h>

using namespace NS_MEDIA;
using namespace NS_MEDIA_GAMING;
using namespace WRL;
using namespace Platform;
using namespace Windows::UI::Xaml::Controls;

Game::Game()
{

}

void Game::SetPresenter(SwapChainPanel ^ swapChainPanel, uint32 width, uint32 height)
{
	auto swapChainPanelUnk = reinterpret_cast<IInspectable*>(swapChainPanel);
	ComPtr<ISwapChainPanelNative> swapChainPanelNative;
	ThrowIfFailed(swapChainPanelUnk->QueryInterface(swapChainPanelNative.GetAddressOf()));
	// 创建设备
	deviceManager = std::make_unique<DeviceManager>(swapChainPanelNative.Get(), width, height);
}
