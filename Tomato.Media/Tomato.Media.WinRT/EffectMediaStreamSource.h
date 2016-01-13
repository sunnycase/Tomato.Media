//
// Tomato Media
// 支持 Effect 的 MediaStreamSource
// 作者：SunnyCase
// 创建时间：2016-01-13
//
#pragma once
#include "common.h"
#include "MediaSource.h"

DEFINE_NS_MEDIA

class EffectMediaStreamSource : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFSourceReaderCallback>
{
public:
	EffectMediaStreamSource(MediaSource^ mediaSource);

	// 通过 RuntimeClass 继承
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) override;
	STDMETHODIMP OnFlush(DWORD dwStreamIndex) override;
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) override;
private:
	void ConfigureSourceReader(IMFMediaSource* mediaSource);
	void InstallEffects();
	void ConfigureMSS();
private:
	Windows::Media::Core::MediaStreamSource^ _mss;
	WRL::ComPtr<IMFSourceReaderEx> _sourceReader;
	WRL::ComPtr<IMFMediaType> _outputMT;
};

END_NS_MEDIA