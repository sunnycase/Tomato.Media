//
// Tomato Media
// [Internal] WASAPI Media Sink
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once
#include "../include/IMediaSink.h"
#include "Utilities/MFMMCSSProvider.h"

NSDEF_TOMATO_MEDIA

// WASAPI Media Sink
class WASAPIMediaSink : public IMediaSink
{
public:
	WASAPIMediaSink();

	virtual concurrency::task<void> Initialize();
	virtual void SetMediaSourceReader(std::shared_ptr<ISourceReader> sourceReader);
	virtual void StartPlayback();
private:
	// 配置设备
	void ConfigureDevice();
	// 获取设备剩余缓冲帧数
	UINT32 GetBufferFramesAvailable();
	// 填充设备剩余缓冲
	void FillBufferAvailable(bool isSilent);
	void FillBufferFromMediaSource(UINT32 framesCount);
	size_t GetBufferFramesPerPeriod();

	// 开始播放命令回调
	void OnStartPlayback();
	// 提供采样请求回调
	void OnSampleRequested();
private:
	wrl::ComPtr<IAudioClient2> audioClient;
	wrl::ComPtr<IAudioRenderClient> renderClient;
	MFMMCSSProvider mcssProvider;
	MediaSinkState sinkState = MediaSinkState::NotInitialized;

	std::unique_ptr<MMCSSThread> startPlaybackThread;
	std::unique_ptr<MMCSSThread> sampleRequestedThread;

	wrl::Wrappers::Event sampleRequestEvent;			// 请求样本事件
	unique_cotaskmem<WAVEFORMATEX> deviceInputFormat;
	REFERENCE_TIME hnsDefaultBufferDuration;
	UINT32 deviceBufferFrames;
	std::recursive_mutex sampleRequestMutex;
	std::shared_ptr<ISourceReader> sourceReaderHolder;
	ISourceReader* sourceReader = nullptr;
};

NSED_TOMATO_MEDIA