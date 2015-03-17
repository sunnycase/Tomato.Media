//
// Tomato Media
// Media Foundation 单流 Audio Source 基类
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "AudioSourceBase.h"
#include "../MFAudioStream.h"

NSDEF_TOMATO_MEDIA

class SingleStreamAudioSourceBase : public AudioSourceBase
{
protected:
	SingleStreamAudioSourceBase();
	virtual ~SingleStreamAudioSourceBase();

	virtual void OnValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);
	virtual concurrency::task<wrl::ComPtr<IMFPresentationDescriptor>>
		OnCreatePresentationDescriptor(wrl::ComPtr<IMFByteStream> stream);
	virtual concurrency::task<std::vector<wrl::ComPtr<IMFMediaType>>>
		OnCreateMediaTypes(wrl::ComPtr<IMFByteStream> stream) = 0;
	virtual void OnStartAudioStream(REFERENCE_TIME position) = 0;
	virtual concurrency::task<void> OnReadSample(wrl::ComPtr<IMFSample> sample) = 0;
	virtual void OnConfigurePresentationDescriptor(IMFPresentationDescriptor *pPD);
	void EndOfDeliver();
private:
	virtual void OnStartStream(DWORD streamId, bool selected, REFERENCE_TIME position);
	virtual concurrency::task<void> OnStreamsRequestData(TOperation& op);
private:
	// 加载音源流
	concurrency::task<void> InitializeAudioStream(wrl::ComPtr<IMFByteStream> stream);
	// 启动音频流
	void StartAudioStream(IMFPresentationDescriptor* pd, REFERENCE_TIME position);

private:
	wrl::ComPtr<MFAudioStream> audioStream;
private:
};

NSED_TOMATO_MEDIA