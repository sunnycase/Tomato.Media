//
// Tomato Media
// IMFSourceReaderCallback 实现
// 作者：SunnyCase
// 创建时间：2015-08-07
//
#pragma once
#include "common.h"
#include <functional>
#include <mfreadwrite.h>

DEFINE_NS_MEDIA

///<summary>IMFSourceReaderCallback 实现</summary>
class MFSourceReaderCallback : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFSourceReaderCallback>
{
public:
	MFSourceReaderCallback(std::function<void(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*)>&& sampleCallback,
		std::function<void(DWORD)>&& flushCallback);

	// 通过 RuntimeClass 继承
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) override;
	STDMETHODIMP OnFlush(DWORD dwStreamIndex) override;
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) override;
private:
	std::function<void(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*)> sampleCallback;
	std::function<void(DWORD)> flushCallback;
};

END_NS_MEDIA