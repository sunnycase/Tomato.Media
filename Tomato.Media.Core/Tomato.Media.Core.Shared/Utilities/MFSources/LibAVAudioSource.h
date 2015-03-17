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
	virtual concurrency::task<void> OnReadSample(wrl::ComPtr<IMFSample> sample);
	virtual void OnConfigurePresentationDescriptor(IMFPresentationDescriptor *pPD);

	void CreateAVFormatContext(wrl::ComPtr<IMFByteStream> stream);
	wrl::ComPtr<IMFMediaType> CreateMediaType();
	void SeekToFrame(uint32_t frameId);
private:
	uint32_t apeStartPosition;			// APE 开始的位置
	wrl::ComPtr<IMFByteStream> stream;		// 音源流
	uint32_t current_frame_id;
	uint32_t current_data_pos;
	std::unique_ptr<MFAVIOContext> avioctx;
	std::shared_ptr<AVFormatContext> avfmtctx;
	AVStream* audioStream = nullptr;
	uint32_t bit_remainder;
};

NSED_TOMATO_MEDIA