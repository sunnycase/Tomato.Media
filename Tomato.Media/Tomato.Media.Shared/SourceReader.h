//
// Tomato Media
// 媒体源读取器
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#pragma once
#include "common.h"
#include <mfreadwrite.h>
#include <d3d11.h>
#include <queue>
#include <concurrent_queue.h>

DEFINE_NS_MEDIA

class SourceReader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::RuntimeClassType::ClassicCom>, IMFSourceReaderCallback>
{
public:
	SourceReader();

	void SetMediaSource(IMFMediaSource* mediaSource);
	void Start();

	// 通过 RuntimeClass 继承
	STDMETHODIMP OnFlush(DWORD dwStreamIndex) override;
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) override;
protected:
	virtual void ConfigureAttributes(IMFAttributes* attributes) {};
	virtual void SetStreamSelection(IMFSourceReader* sourceReader) = 0;

	void OnEndOfStream();
	// 发送采样请求
	void PostSampleRequest();
private:
	void InitializeSourceReader(IMFMediaSource* mediaSource);
	void ConfigureSourceReader();
protected:
	WRL::ComPtr<IMFSourceReader> sourceReader;
	volatile bool isActive;
};

struct USIZE
{
	UINT32 Width;
	UINT32 Height;
};

#if (WINVER >= _WIN32_WINNT_WIN7)

class VideoSourceReader : public SourceReader
{
public:
	const size_t DefaultVideoCacheTime = 3;

	VideoSourceReader();

	bool TryReadVideoSample(WRL::ComPtr<IMFSample>& sample);
	concurrency::task<WRL::ComPtr<IMFSample>> ReadVideoSampleAsync();

	DEFINE_PROPERTY_GET(OutputMediaType, IMFMediaType*);
	IMFMediaType* get_OutputMediaType() const noexcept { return outputMT.Get(); }
	DEFINE_PROPERTY_GET(FrameSize, USIZE);
	USIZE get_FrameSize() const { return frameSize; }

	// 通过 RuntimeClass 继承
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) override;
protected:
	virtual void ConfigureAttributes(IMFAttributes* attributes) override;
	virtual void SetStreamSelection(IMFSourceReader* sourceReader) override;
private:
	void ConfigureOutputMediaType(IMFSourceReader* sourceReader);
	void InitializeDXGIDeviceManager(IDXGIAdapter* dxgiAdapter);
	void DispatchIncomingSample(IMFSample* pSample);
	void PostSampleRequestIfNeed();
private:
	std::queue<WRL::ComPtr<IMFSample>> videoCache;
	concurrency::concurrent_queue<concurrency::task_completion_event<WRL::ComPtr<IMFSample>>> sampleRequest;
	USIZE frameSize = { 0 };
	size_t videoCacheSize;
	WRL::ComPtr<IMFMediaType> outputMT;
	UINT resetToken;
	std::recursive_mutex videoCacheMutex;
	std::mutex videoCacheStarveCondMutex;
	std::condition_variable videoCacheStarveCond;
	WRL::ComPtr<IMFDXGIDeviceManager> dxgiDeviceManager;
};

#endif

END_NS_MEDIA