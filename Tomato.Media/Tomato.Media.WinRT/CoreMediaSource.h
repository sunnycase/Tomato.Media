//
// Tomato Media
// Windows::Meida::Core::IMediaSource 实现
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#pragma once
#include "common.h"
#include <windows.media.core.h>
#include <mfidl.h>
#include "MediaSource.h"

DEFINE_NS_MEDIA

class CoreMediaSource sealed : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::RuntimeClassType::WinRtClassicComMix>,
	ABI::Windows::Media::Core::IMediaSource, IMFGetService>
{
public:
	CoreMediaSource(MediaSource^ mediaSource);
	virtual ~CoreMediaSource();

	// 通过 RuntimeClass 继承
	STDMETHODIMP GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject) override;
private:
	MediaSource^ mediaSource;
};

END_NS_MEDIA