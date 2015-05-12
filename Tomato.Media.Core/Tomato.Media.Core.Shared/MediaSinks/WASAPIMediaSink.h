//
// Tomato Media
// [Internal] WASAPI Media Sink
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once
#include "../include/IMediaSink.h"
#include "Utilities/WorkerQueueProvider.h"

NSDEF_TOMATO_MEDIA

// WASAPI Media Sink
class WASAPIMediaSink : public IMediaSink
{
public:
	WASAPIMediaSink(IMediaSinkHandler& handler);

	virtual void SetMediaSourceReader(std::shared_ptr<ISourceReader> sourceReader);
	virtual MediaSinkStatus GetCurrentStatus() const noexcept { return sinkState; }
	virtual void Start();
	virtual void Pause();
	virtual void Stop();
	virtual int64_t GetPosition() const;
	virtual void SetPosition(int64_t position);
	virtual int64_t GetDuration() const;
	virtual double GetVolume();
	virtual void SetVolume(double volume);
private:
	concurrency::task<void> InitializeDevice();
	// 配置设备
	void ConfigureDevice();
	// 获取设备剩余缓冲帧数
	inline UINT32 GetBufferFramesAvailable();
	// 填充设备剩余缓冲
	void FillBufferAvailable(bool isSilent);
	void FillBufferFromMediaSource(UINT32 framesCount);
	size_t GetBufferFramesPerPeriod();
	void InitializeDeviceBuffer();

	// 开始播放命令回调
	void OnStart();
	void OnSeek();
	void OnPause();
	void OnStop();
	void OnMediaEnded();
	// 提供采样请求回调
	void OnSampleRequested();
	void SetStatus(MediaSinkStatus status);
	void ReportOnError(concurrency::task<void> task);
private:
	wrl::ComPtr<IAudioClient2> audioClient;
	wrl::ComPtr<IAudioRenderClient> renderClient;
	wrl::ComPtr<ISimpleAudioVolume> simpleAudioVolume;
	std::unique_ptr<WorkerQueueProvider> mcssProvider;
	MediaSinkStatus sinkState = MediaSinkStatus::Closed;
	// 加载设备任务
	concurrency::task<void> loadDeviceTask;

	std::unique_ptr<WorkerThread> startThread;
	std::unique_ptr<WorkerThread> seekThread;
	std::unique_ptr<WorkerThread> setMediaSourceReaderThread;
	std::unique_ptr<WorkerThread> pauseThread;
	std::unique_ptr<WorkerThread> stopThread;
	std::unique_ptr<WorkerThread> sampleRequestedThread;

	wrl::Wrappers::Event sampleRequestEvent;			// 请求样本事件
	unique_cotaskmem<WAVEFORMATEX> deviceInputFormat;
	REFERENCE_TIME hnsDefaultBufferDuration;
	UINT32 deviceBufferFrames;
	wrl::Wrappers::CriticalSection stateMonitor;
	std::shared_ptr<ISourceReader> sourceReaderHolder;
	ISourceReader* sourceReader = nullptr;
	uint32_t currentFrames = 0;
	IMediaSinkHandler& sinkHandler;
	bool seeking = false;
	int64_t seekPosition = 0;
};

NSED_TOMATO_MEDIA