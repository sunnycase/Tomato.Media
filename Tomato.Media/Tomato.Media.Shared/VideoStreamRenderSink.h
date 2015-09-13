//
// Tomato Media
// 视频流渲染 Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#pragma once
#include "IVideoRender.h"
#include "StreamRenderSinkBase.h"
#include "../../include/MFWorkerQueueProvider.h"
#include <atomic>
#include <chrono>
#include <queue>

DEFINE_NS_MEDIA

///<summary>视频流 Sink</summary>
class VideoStreamRenderSink : public StreamRenderSinkBase
{
	// Sink 状态
	enum VideoStreamRenderSinkState
	{
		// 未加载（未设置媒体类型）
		NotInitialized,
		// 已加载（已设置媒体类型，且 Flush 后）
		Initialized,
		// 缓冲中
		Prerolling,
		// 准备完毕（缓冲完毕）
		Ready,
		// 播放中
		Playing
	};

	struct FrameInfo : public Frame
	{
		MFTIME SampleTime;
		MFTIME Duration;

		FrameInfo() : SampleTime(0), Duration(0) {}

		FrameInfo(Frame frame, MFTIME sampleTime, MFTIME duration)
			:Frame(std::move(frame)), SampleTime(sampleTime), Duration(duration)
		{

		}
	};

	const MFTIME FrameCacheDuration = MFCLOCK_FREQUENCY_HNS * 3;
public:
	VideoStreamRenderSink(DWORD identifier, MediaRenderSink* mediaSink, IVideoRender* videoRender);

	STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler ** ppHandler) override;
	STDMETHODIMP ProcessSample(IMFSample * pSample) override;
	STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT * pvarMarkerValue, const PROPVARIANT * pvarContextValue) override;
	STDMETHODIMP Flush(void) override;

	STDMETHODIMP IsMediaTypeSupported(IMFMediaType * pMediaType, IMFMediaType ** ppMediaType) override;
	STDMETHODIMP GetMediaTypeCount(DWORD * pdwTypeCount) override;
	STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType ** ppType) override;
	STDMETHODIMP SetCurrentMediaType(IMFMediaType * pMediaType) override;
	STDMETHODIMP GetCurrentMediaType(IMFMediaType ** ppMediaType) override;
	STDMETHODIMP GetMajorType(GUID * pguidMajorType) override;

#if (WINVER >= _WIN32_WINNT_WIN8)
	STDMETHODIMP RegisterThreadsEx(DWORD * pdwTaskIndex, LPCWSTR wszClassName, LONG lBasePriority) override;
	STDMETHODIMP SetWorkQueueEx(DWORD dwMultithreadedWorkQueueId, LONG lWorkItemBasePriority) override;
#elif (WINVER >= _WIN32_WINNT_VISTA) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	STDMETHODIMP RegisterThreads(DWORD dwTaskIndex, LPCWSTR wszClass) override;
	STDMETHODIMP SetWorkQueue(DWORD dwWorkQueueId) override;
#endif
	STDMETHODIMP UnregisterThreads(void) override;

	virtual void SetPresentationClock(IMFPresentationClock* presentationClock) override;
	virtual void NotifyPreroll(MFTIME hnsUpcomingStartTime) override;
	virtual void Play(MFTIME startTime) override;
private:
	void OnSetMediaType();
	///<param name="setInited">是否设置状态为 Initialized。</param>
	///<remarks>调用前需对状态加锁</remarks>
	void FlushCore(bool setInited = false);

	///<remarks>调用前需对状态加锁</remarks>
	void PostSampleRequest();

	///<remarks>调用前需对状态加锁</remarks>
	void PostSampleRequestIfNeeded();

	///<remarks>调用前无需对状态加锁</remarks>
	void OnProcessIncomingSamples(IMFSample* sample);

	///<remarks>调用前需对状态加锁</remarks>
	void RegisterWorkThreadIfNeeded();

	///<remarks>调用前无需对状态加锁</remarks>
	void RequestDecodeFrame();

	///<remarks>调用前无需对状态加锁</remarks>
	void RequestRenderFrame();

	///<summary>将缓存的采样解码为帧</summary>
	///<remarks>
	/// 调用前不能对状态加锁，保证同时只有一个线程调用。
	/// 若发生异常，保证 cachedFrameDuration 有效，无法解码的采样将被丢弃。
	///</remarks>
	void OnDecodeFrame();

	///<summary>将帧渲染出来</summary>
	///<remarks>调用前不能对状态加锁，保证同时只有一个线程调用</remarks>
	void OnRenderFrame();

	WRL::ComPtr<IMFSample> TryPopSample();
private:
	UINT32 frameWidth, frameHeight;
	WRL::ComPtr<IVideoRender> videoRender;
	WRL::ComPtr<IMFMediaType> mediaType;
	std::mutex stateMutex;
	VideoStreamRenderSinkState sinkState = NotInitialized;
	std::queue<WRL::ComPtr<IMFSample>> sampleCache;
	std::queue<FrameInfo> frameCache;
	std::mutex sampleCacheMutex;
	std::mutex frameCacheMutex;
	Core::MFWorkerQueueProviderRef workerQueue;
	bool workThreadRegistered = false;

	std::shared_ptr<Core::WorkerThread> decodeFrameWorker;
	std::atomic<bool> decodeFrameWorkerActived = false;
	std::atomic<MFTIME> cachedFrameDuration = 0;
	std::atomic<bool> streamEnded = false;

	std::shared_ptr<Core::WorkerThread> renderFrameWorker;
	std::atomic<bool> renderFrameWorkerActived = false;

	WRL::ComPtr<IMFPresentationClock> presentationClock;
};

END_NS_MEDIA