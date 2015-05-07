//
// Tomato Media
// [Internal] Windows.Media.Core.IMediaSource 实现
// 
// (c) SunnyCase 
// 创建日期 2015-05-07
#pragma once
#include "../../include/platform.h"
#include <windows.media.core.h>

NSDEF_TOMATO_MEDIA

class WindowsCoreMediaSource : public wrl::RuntimeClass<wrl::RuntimeClassFlags<
	wrl::RuntimeClassType::WinRtClassicComMix>, ABI::Windows::Media::Core::IMediaSource,
	IMFGetService>
{
public:
	WindowsCoreMediaSource();

	// 通过 RuntimeClass 继承
	STDMETHODIMP GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject) override;
private:
	wrl::ComPtr<IMFMediaSource> OnCreateMediaSource();
};

NSED_TOMATO_MEDIA