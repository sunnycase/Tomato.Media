//
// Tomato Media Gaming
// 游戏主体
// 
// 作者：SunnyCase 
// 创建日期 2015-09-05
#pragma once
#include "common.h"
#include "DeviceManager.h"

DEFINE_NS_MEDIA_GAMING

///<summary>游戏主体</summary>
[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class Game sealed
{
public:
	Game();

	void SetPresenter(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel, uint32 width, uint32 height);
internal:
	property DeviceManager& DeviceManager
	{
		Gaming::DeviceManager& get() { return *deviceManager; }
	}
private:
	std::unique_ptr<Gaming::DeviceManager> deviceManager;
};

END_NS_MEDIA_GAMING