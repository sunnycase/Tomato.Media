//
// Tomato Media
// 视频流渲染 Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#include "pch.h"
#include "VideoStreamRenderSink.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

namespace
{
	const std::array<GUID, 1> supportedSubTypes = {
		MFVideoFormat_NV12
	};
}

#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> locker(stateMutex)
#define CHECK_INITED() if(sinkState == NotInitialized) return MF_E_NOT_INITIALIZED
#define CHECK_INITED_THROW() if(sinkState == NotInitialized) ThrowIfFailed(MF_E_NOT_INITIALIZED)

VideoStreamRenderSink::VideoStreamRenderSink(DWORD identifier, MediaRenderSink* mediaSink, IVideoRender* videoRender)
	:StreamRenderSinkBase(identifier, mediaSink), videoRender(videoRender), workerQueue(MFASYNC_CALLBACK_QUEUE_UNDEFINED)
{
}

HRESULT VideoStreamRenderSink::GetMediaTypeHandler(IMFMediaTypeHandler ** ppHandler)
{
	*ppHandler = this;
	return S_OK;
}

HRESULT VideoStreamRenderSink::ProcessSample(IMFSample * pSample)
{
	if (!pSample) return E_INVALIDARG;
	try
	{
		{
			LOCK_STATE();
			CHECK_INITED_THROW();
			RegisterWorkThreadIfNeeded();
		}

		OnProcessIncomingSamples(pSample);
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT VideoStreamRenderSink::PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT * pvarMarkerValue, const PROPVARIANT * pvarContextValue)
{
	return S_OK;
}

HRESULT VideoStreamRenderSink::Flush(void)
{
	LOCK_STATE();
	CHECK_INITED();
	try
	{
		FlushCore(false);
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT VideoStreamRenderSink::IsMediaTypeSupported(IMFMediaType * pMediaType, IMFMediaType ** ppMediaType)
{
	try
	{
		GUID majorType;
		ThrowIfFailed(pMediaType->GetMajorType(&majorType));

		if (majorType == MFMediaType_Video)
		{

			GUID subType;
			ThrowIfFailed(pMediaType->GetGUID(MF_MT_SUBTYPE, &subType));
			auto it = std::find(supportedSubTypes.begin(), supportedSubTypes.end(), subType);
			if (it != supportedSubTypes.end())
				return S_OK;
			else if (ppMediaType)
			{
				ComPtr<IMFMediaType> desiredMediaType;
				ThrowIfFailed(MFCreateMediaType(&desiredMediaType));
				ThrowIfFailed(desiredMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
				ThrowIfFailed(desiredMediaType->SetGUID(MF_MT_SUBTYPE, supportedSubTypes.front()));
				ThrowIfFailed(desiredMediaType.CopyTo(ppMediaType));
			}
		}
		return MF_E_INVALIDMEDIATYPE;
	}
	CATCH_ALL();

	return S_OK;
}

HRESULT VideoStreamRenderSink::GetMediaTypeCount(DWORD * pdwTypeCount)
{
	*pdwTypeCount = supportedSubTypes.size();
	return S_OK;
}

HRESULT VideoStreamRenderSink::GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType ** ppType)
{
	if (dwIndex >= supportedSubTypes.size())
		return MF_E_NO_MORE_TYPES;

	try
	{
		ComPtr<IMFMediaType> desiredMediaType;
		ThrowIfFailed(MFCreateMediaType(&desiredMediaType));
		ThrowIfFailed(desiredMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		ThrowIfFailed(desiredMediaType->SetGUID(MF_MT_SUBTYPE, supportedSubTypes[dwIndex]));
		return desiredMediaType.CopyTo(ppType);
	}
	CATCH_ALL();
}

HRESULT VideoStreamRenderSink::SetCurrentMediaType(IMFMediaType * pMediaType)
{
	auto hr = S_OK;
	try
	{
		LOCK_STATE();
		if (sinkState != NotInitialized) return E_NOT_VALID_STATE;
		if (mediaType.Get() != pMediaType)
		{
			hr = IsMediaTypeSupported(pMediaType, nullptr);
			if (SUCCEEDED(hr))
			{
				mediaType = pMediaType;
				OnSetMediaType();
			}
		}
	}
	CATCH_ALL();
	return hr;
}

HRESULT VideoStreamRenderSink::GetCurrentMediaType(IMFMediaType ** ppMediaType)
{
	try
	{
		LOCK_STATE();
		if (mediaType)
			return mediaType.CopyTo(ppMediaType);
		return MF_E_NOT_INITIALIZED;
	}
	CATCH_ALL();
}

HRESULT VideoStreamRenderSink::GetMajorType(GUID * pguidMajorType)
{
	*pguidMajorType = MFMediaType_Video;
	return S_OK;
}

void VideoStreamRenderSink::NotifyPreroll(MFTIME hnsUpcomingStartTime)
{
	LOCK_STATE();
	CHECK_INITED_THROW();

	// 状态必须为尚未开始缓冲
	if (sinkState != Initialized)
		ThrowIfFailed(E_NOT_VALID_STATE);
	sinkState = Prerolling;
	PostSampleRequest();
}

void VideoStreamRenderSink::OnSetMediaType()
{
	videoRender->Initialize();
	ThrowIfFailed(MFGetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, &frameWidth, &frameHeight));

	FlushCore(true);
}

void VideoStreamRenderSink::FlushCore(bool setInited)
{
	if (setInited)
	{
		assert(sinkState == NotInitialized);
		sinkState = Initialized;
	}
}

void VideoStreamRenderSink::PostSampleRequest()
{
	CHECK_INITED_THROW();

	ThrowIfFailed(eventQueue->QueueEventParamVar(MEStreamSinkRequestSample, GUID_NULL, S_OK, nullptr));
}

void VideoStreamRenderSink::PostSampleRequestIfNeeded()
{
	CHECK_INITED_THROW();

	std::lock_guard<decltype(sampleCacheMutex)> locker(sampleCacheMutex);
	if (sampleCache.empty() && !streamEnded)
		PostSampleRequest();
}

void VideoStreamRenderSink::OnProcessIncomingSamples(IMFSample* sample)
{
	if (!sample) ThrowIfFailed(E_INVALIDARG);

	{
		std::lock_guard<decltype(sampleCacheMutex)> locker(sampleCacheMutex);
		sampleCache.emplace(sample);
	}
	RequestDecodeFrame();
}

void VideoStreamRenderSink::RegisterWorkThreadIfNeeded()
{
	if (workThreadRegistered) return;

	if (!workerQueue.IsValid())
		workerQueue = MFWorkerQueueProvider::GetAudio();

	auto weak(AsWeak());
	decodeFrameWorker = workerQueue.CreateWorkerThread([weak] { if (auto me = weak.Resolve()) ((VideoStreamRenderSink*)me.Get())->OnDecodeFrame(); });
	renderFrameWorker = workerQueue.CreateWorkerThread([weak] { if (auto me = weak.Resolve()) ((VideoStreamRenderSink*)me.Get())->OnRenderFrame(); });
	workThreadRegistered = true;
}

void VideoStreamRenderSink::RequestDecodeFrame()
{
	bool expected = false;
	if (decodeFrameWorkerActived.compare_exchange_strong(expected, true))
		decodeFrameWorker->Execute();
}

void VideoStreamRenderSink::OnDecodeFrame()
{
	try
	{
		{
			LOCK_STATE();
			if(sinkState != Prerolling)
				return decodeFrameWorkerActived.store(false, std::memory_order_release);
			// 如果状态为正在缓冲且缓冲完毕则发送事件
			else if (sinkState == Prerolling && cachedFrameDuration >= FrameCacheDuration)
			{
				ThrowIfFailed(eventQueue->QueueEventParamVar(MEStreamSinkPrerolled, GUID_NULL, S_OK, nullptr));
				sinkState = Ready;
				return decodeFrameWorkerActived.store(false, std::memory_order_release);
			}
		}

		// 取出采样
		ComPtr<IMFSample> sample;
		{
			{
				std::lock_guard<decltype(sampleCacheMutex)> locker(sampleCacheMutex);
				if (!sampleCache.empty())
				{
					sample = std::move(sampleCache.front());
					sampleCache.pop();
				}
			}
			LOCK_STATE();
			PostSampleRequestIfNeeded();
		}
		if (sample)
		{
			MFTIME duration;
			ThrowIfFailed(sample->GetSampleDuration(&duration));

			auto frame = videoRender->CreateFrame(sample.Get(), frameWidth, frameHeight);
			{
				std::lock_guard<decltype(frameCacheMutex)> locker(frameCacheMutex);
				frameCache.emplace(std::move(frame));
			}
			cachedFrameDuration += duration;
		}

		// 没读到 sample 则暂时退出线程
		if (cachedFrameDuration < FrameCacheDuration && !streamEnded && sample)
			decodeFrameWorker->Execute();
		else
			decodeFrameWorkerActived.store(false, std::memory_order_release);
	}
	catch (...)
	{
		decodeFrameWorkerActived.store(false, std::memory_order_release);
		throw;
	}
}

void VideoStreamRenderSink::OnRenderFrame()
{
	Frame frame;
	{
		{
			std::lock_guard<decltype(frameCacheMutex)> locker(frameCacheMutex);
			if (!frameCache.empty())
			{
				frame = std::move(frameCache.front());
				frameCache.pop();
			}
		}
		RequestDecodeFrame();
	}
	if (frame.Luminance && frame.Chrominance)
	{
		videoRender->RenderFrame(frame);
	}
}

#if (WINVER >= _WIN32_WINNT_WIN8)

HRESULT VideoStreamRenderSink::RegisterThreadsEx(DWORD * pdwTaskIndex, LPCWSTR wszClassName, LONG lBasePriority)
{
	return E_NOTIMPL;
}

HRESULT VideoStreamRenderSink::SetWorkQueueEx(DWORD dwMultithreadedWorkQueueId, LONG lWorkItemBasePriority)
{
	return E_NOTIMPL;
}

#elif (WINVER >= _WIN32_WINNT_VISTA) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

HRESULT VideoStreamRenderSink::RegisterThreads(DWORD dwTaskIndex, LPCWSTR wszClass)
{
	return E_NOTIMPL;
}

HRESULT VideoStreamRenderSink::SetWorkQueue(DWORD dwWorkQueueId)
{
	return E_NOTIMPL;
}

#endif

HRESULT VideoStreamRenderSink::UnregisterThreads(void)
{
	return E_NOTIMPL;
}
