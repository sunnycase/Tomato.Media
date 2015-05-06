//
// Tomato Media
// Media Foundation LibAV Audio Source
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "SingleStreamAudioSourceBase.h"
#include "../libavhelpers.h"

NSDEF_TOMATO_MEDIA

class LibAVAudioSource : public SingleStreamAudioSourceBase
{
public:
	LibAVAudioSource();
	virtual ~LibAVAudioSource();
private:
	virtual DWORD OnGetCharacteristics();
	virtual concurrency::task<std::vector<wrl::ComPtr<IMFMediaType>>>
		OnCreateMediaTypes(wrl::ComPtr<IMFByteStream> stream);
	virtual void OnStartAudioStream(REFERENCE_TIME position);
	virtual concurrency::task<bool> OnReadSample(wrl::ComPtr<IMFSample> sample);
	virtual void OnConfigurePresentationDescriptor(IMFPresentationDescriptor *pPD);

	void CreateAVFormatContext(wrl::ComPtr<IMFByteStream> stream);
	wrl::ComPtr<IMFMediaType> CreateMediaType();
	void SeekToFrame(uint32_t frameId);
private:
	std::unique_ptr<MFAVIOContext> avioctx;
	std::shared_ptr<AVFormatContext> avfmtctx;
	AVStream* audioStream = nullptr;
};

NSED_TOMATO_MEDIA