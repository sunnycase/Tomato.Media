//
// Tomato Media Codec
// 负责发送数据的 Media Stream 基类
// 
// 作者：SunnyCase
// 创建时间：2015-09-09
#include "pch.h"
#include "MediaSources/MediaSourceBase.h"
#include "DeliverMediaStreamBase.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#define LOCK_SAMPLE_REQUESTS() std::lock_guard<decltype(sampleRequestsMutex)> locker(sampleRequestsMutex)
#define LOCK_SAMPLES_CACHE() std::lock_guard<decltype(samplesCacheMutex)> locker(samplesCacheMutex)
#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> stateLocker(stateMutex)

DeliverMediaStreamBase::DeliverMediaStreamBase(NS_CORE::WeakRef<MediaSourceBase> mediaSource, IMFStreamDescriptor* streamDescriptor)
	:mediaSource(mediaSource), streamDescriptor(streamDescriptor)
{
	ThrowIfFailed(MFCreateEventQueue(&eventQueue));
}

DeliverMediaStreamBase::~DeliverMediaStreamBase()
{
}

HRESULT DeliverMediaStreamBase::GetEvent(DWORD dwFlags, IMFMediaEvent ** ppEvent)
{
	return eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT DeliverMediaStreamBase::BeginGetEvent(IMFAsyncCallback * pCallback, IUnknown * punkState)
{
	return eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT DeliverMediaStreamBase::EndGetEvent(IMFAsyncResult * pResult, IMFMediaEvent ** ppEvent)
{
	return eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT DeliverMediaStreamBase::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT * pvValue)
{
	return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

HRESULT DeliverMediaStreamBase::GetMediaSource(IMFMediaSource ** ppMediaSource)
{
	auto mediaSource = this->mediaSource.Resolve();
	if (!mediaSource) return MF_E_SHUTDOWN;
	return mediaSource.CopyTo(ppMediaSource);
}

HRESULT DeliverMediaStreamBase::GetStreamDescriptor(IMFStreamDescriptor ** ppStreamDescriptor)
{
	return streamDescriptor.CopyTo(ppStreamDescriptor);
}

HRESULT DeliverMediaStreamBase::RequestSample(IUnknown * pToken)
{
	try
	{
		{
			LOCK_STATE();
			if (streamState == Stopped)
				return MF_E_INVALIDREQUEST;
			else if (streamState == EndOfStream)
				return MF_E_END_OF_STREAM;

			LOCK_SAMPLE_REQUESTS();
			sampleRequests.emplace(pToken);
		}
		DispatchSampleRequests();
	}
	CATCH_ALL();
	return S_OK;
}

bool DeliverMediaStreamBase::DoesNeedMoreData()
{
	if (!IsActive())
		return false;
	if (cachedDuration != 0)
		return !endOfDeliver && cachedDuration < DesiredCacheDuration;
	else
	{
		LOCK_SAMPLES_CACHE();
		return !endOfDeliver && samplesCache.size() < DesiredCacheSamples;
	}
}

void DeliverMediaStreamBase::Start(const PROPVARIANT & position)
{
	{
		LOCK_STATE();
		if ((streamState == Started ||
			streamState == Paused) && position.vt != VT_EMPTY)
		{
			std::lock(sampleRequestsMutex, samplesCacheMutex);
			std::lock_guard<decltype(sampleRequestsMutex)> locker(sampleRequestsMutex, std::adopt_lock);
			std::lock_guard<decltype(samplesCacheMutex)> locker2(samplesCacheMutex, std::adopt_lock);

			samplesCache.swap(decltype(samplesCache)());
			sampleRequests.swap(decltype(sampleRequests)());
		}

		auto met = IsActive() ? MEStreamSeeked : MEStreamStarted;
		streamState = Started;
		endOfDeliver.store(false, std::memory_order_release);
		ThrowIfFailed(eventQueue->QueueEventParamVar(met, GUID_NULL, S_OK, &position));
	}
	DispatchSampleRequests();
}

void DeliverMediaStreamBase::Pause()
{
	LOCK_STATE();
	if (streamState == Started)
	{
		streamState = Paused;
		ThrowIfFailed(eventQueue->QueueEventParamVar(MEStreamPaused, GUID_NULL, S_OK, nullptr));
	}
}

void DeliverMediaStreamBase::Stop()
{
	LOCK_STATE();
	if (streamState != Stopped)
	{
		std::lock(sampleRequestsMutex, samplesCacheMutex);
		std::lock_guard<decltype(sampleRequestsMutex)> locker(sampleRequestsMutex, std::adopt_lock);
		std::lock_guard<decltype(samplesCacheMutex)> locker2(samplesCacheMutex, std::adopt_lock);

		samplesCache.swap(decltype(samplesCache)());
		sampleRequests.swap(decltype(sampleRequests)());

		streamState = Stopped;
		ThrowIfFailed(eventQueue->QueueEventParamVar(MEStreamStopped, GUID_NULL, S_OK, nullptr));
	}
}

void DeliverMediaStreamBase::EndOfDeliver()
{
	endOfDeliver.store(true, std::memory_order_release);
}

void DeliverMediaStreamBase::EnqueueSample(IMFSample* sample)
{
	if (!sample) ThrowIfFailed(E_INVALIDARG);

	MFTIME duration;
	{
		LOCK_SAMPLES_CACHE();
		samplesCache.emplace(sample);
	}
	if (SUCCEEDED(sample->GetSampleDuration(&duration)))
		cachedDuration += duration;
	DispatchSampleRequests();
}

void DeliverMediaStreamBase::DispatchSampleRequests()
{
	{
		LOCK_STATE();
		if (streamState != Started)return;
	}
	bool endOfStream = false;
	{
		std::lock(sampleRequestsMutex, samplesCacheMutex);
		std::lock_guard<decltype(sampleRequestsMutex)> locker(sampleRequestsMutex, std::adopt_lock);
		std::lock_guard<decltype(samplesCacheMutex)> locker2(samplesCacheMutex, std::adopt_lock);

		while (!sampleRequests.empty() && !samplesCache.empty())
		{
			LOCK_STATE();
			if (streamState != Started)break;

			auto token = std::move(sampleRequests.front());
			sampleRequests.pop();

			auto sample = std::move(samplesCache.front());
			samplesCache.pop();
			ThrowIfFailed(sample->SetUnknown(MFSampleExtension_Token, token.Get()));
			ThrowIfFailed(eventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, S_OK, sample.Get()));
			MFTIME duration;
			if (SUCCEEDED(sample->GetSampleDuration(&duration)))
				cachedDuration -= duration;
		}

		if (samplesCache.empty() && endOfDeliver)
			endOfStream = true;
	}
	if (endOfStream)
		OnEndOfStream();
	RequestDataIfNeeded();
}

void DeliverMediaStreamBase::OnEndOfStream()
{
	streamState = EndOfStream;
	ThrowIfFailed(eventQueue->QueueEventParamVar(MEEndOfStream, GUID_NULL, S_OK, nullptr));

	// 提示媒体源本流结束
	if (auto mediaSource = this->mediaSource.Resolve())
		mediaSource->QueueAsyncOperation(MediaSourceOperationKind::EndOfStream);
}

void DeliverMediaStreamBase::RequestData()
{
	if (auto mediaSource = this->mediaSource.Resolve())
		mediaSource->QueueAsyncOperation(std::make_shared<MediaStreamRequestDataOperation>(this));
}


void DeliverMediaStreamBase::RequestDataIfNeeded()
{
	if (DoesNeedMoreData())
		RequestData();
}
