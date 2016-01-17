//
// Tomato Media
// 支持 Effect 的 MediaStreamSource
// 作者：SunnyCase
// 创建时间：2016-01-13
//
#include "pch.h"
#include "EffectMediaStreamSource.h"
#include "Utility/MFSourceReaderCallback.h"
#include "../../include/Wrappers.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage::Streams;
using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace WRL;
using namespace concurrency;

EffectMediaStreamSource::EffectMediaStreamSource(::NS_MEDIA::MediaSource ^ mediaSource)
{
	ConfigureSourceReader(mediaSource->MFMediaSource);
	ConfigureMSS();
}

void EffectMediaStreamSource::AddTransform(Windows::Media::IMediaExtension ^ transform)
{
	auto obj = reinterpret_cast<IInspectable*>(static_cast<Object^>(transform));
	ComPtr<IMFTransform> mfTrans;
	ThrowIfFailed(obj->QueryInterface(IID_PPV_ARGS(&mfTrans)));
	task_completion_event<void> flushOp;
	{
		std::lock_guard<decltype(_flushOperationsMutex)> locker(_flushOperationsMutex);
		_flushOperations.emplace(flushOp);
	}
	create_task(flushOp).then([this, mfTrans]
	{
		_sourceReader->AddTransformForStream(MF_SOURCE_READER_FIRST_AUDIO_STREAM, mfTrans.Get());
	});
	ThrowIfFailed(_sourceReader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM));
}

void EffectMediaStreamSource::RemoveAllTransform()
{
	task_completion_event<void> flushOp;
	{
		std::lock_guard<decltype(_flushOperationsMutex)> locker(_flushOperationsMutex);
		_flushOperations.emplace(flushOp);
	}
	create_task(flushOp).then([this]
	{
		_sourceReader->RemoveAllTransformsForStream(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
	});
	ThrowIfFailed(_sourceReader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM));
}

void EffectMediaStreamSource::ConfigureSourceReader(IMFMediaSource * mediaSource)
{
	ComPtr<IMFAttributes> attributes;
	ThrowIfFailed(MFCreateAttributes(&attributes, 3));
	// 设置 Callback
	WeakReference weak(this);
	auto callback = Make<MFSourceReaderCallback>([weak](HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
	{
		if (auto me = weak.Resolve<EffectMediaStreamSource>())
			me->OnReadSample(hrStatus, dwStreamIndex, dwStreamFlags, llTimestamp, pSample);
	}, [weak](DWORD dwStreamIndex)
	{
		if (auto me = weak.Resolve<EffectMediaStreamSource>())
			me->OnFlush(dwStreamIndex);
	});
	ThrowIfFailed(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callback.Get()));
	ThrowIfFailed(attributes->SetString(MF_READWRITE_MMCSS_CLASS_AUDIO, L"Audio"));
	ThrowIfFailed(attributes->SetUINT32(MF_READWRITE_MMCSS_PRIORITY_AUDIO, 4));

	ComPtr<IMFSourceReader> sourceReader;
	ThrowIfFailed(MFCreateSourceReaderFromMediaSource(mediaSource, attributes.Get(), &sourceReader));
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false));
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true));
	ThrowIfFailed(sourceReader.As(&_sourceReader));

	ComPtr<IMFMediaType> mediaType;

	ThrowIfFailed(MFCreateMediaType(&mediaType));
	ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
	ThrowIfFailed(sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaType.Get()));
	InstallEffects();
	ThrowIfFailed(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &_outputMT));
}

void EffectMediaStreamSource::InstallEffects()
{
}

void EffectMediaStreamSource::ConfigureMSS()
{
	UINT32 sampleRate, channelCount, bitsPerSample;
	ThrowIfFailed(_outputMT->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate));
	ThrowIfFailed(_outputMT->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount));
	ThrowIfFailed(_outputMT->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
	auto encProps = AudioEncodingProperties::CreatePcm(sampleRate, channelCount, bitsPerSample);
	encProps->Subtype = L"Float";
	auto streamDesc = ref new AudioStreamDescriptor(encProps);
	_mss = ref new MediaStreamSource(streamDesc);

	PROPVARIANT duration;
	PropVariantInit(&duration);
	auto fin = make_finalizer([&] {PropVariantClear(&duration);});
	if (SUCCEEDED(_sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &duration)))
		_mss->Duration = TimeSpan{ duration.hVal.QuadPart };
	_mss->CanSeek = true;
	_mss->Starting += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^>(this, &EffectMediaStreamSource::OnStarting);
	_mss->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(this, &EffectMediaStreamSource::OnSampleRequested);
	_mss->Closed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, Windows::Media::Core::MediaStreamSourceClosedEventArgs ^>(this, &EffectMediaStreamSource::OnClosed);
}

void EffectMediaStreamSource::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	if (SUCCEEDED(hrStatus))
	{
		if (dwStreamFlags & MF_SOURCE_READERF_ERROR ||
			dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
			_endOfStream.store(true, std::memory_order_release);

		if (pSample)
		{
			std::lock_guard<decltype(_sampleCacheMutex)> sampleLocker(_sampleCacheMutex);
			_sampleCache.emplace(pSample);
		}
		DispatchSampleRequests();
	}
	else
		_mss->NotifyError(MediaStreamSourceErrorStatus::DecodeError);
}

void EffectMediaStreamSource::OnFlush(DWORD dwStreamIndex)
{
	std::lock_guard<decltype(_flushOperationsMutex)> locker(_flushOperationsMutex);
	if (!_flushOperations.empty())
	{
		auto operation = std::move(_flushOperations.front());
		_flushOperations.pop();
		operation.set();
	}
}

void EffectMediaStreamSource::DispatchSampleRequests()
{
	bool needMore = false;
	{
		std::unique_lock<decltype(_requestCacheMutex)> requestLocker(_requestCacheMutex, std::defer_lock);
		std::unique_lock<decltype(_sampleCacheMutex)> sampleLocker(_sampleCacheMutex, std::defer_lock);
		std::lock(requestLocker, sampleLocker);

		while (!_requestCache.empty() && !_sampleCache.empty())
		{
			auto mfSample = _sampleCache.front();
			auto request = std::move(_requestCache.front());
			MFTIME timestamp = -1, duration = 0;
			mfSample->GetSampleTime(&timestamp);
			mfSample->GetSampleDuration(&duration);

			ComPtr<IMFMediaBuffer> buffer;
			ThrowIfFailed(mfSample->ConvertToContiguousBuffer(&buffer));
			ComPtr<ABI::Windows::Storage::Streams::IBuffer> rtBuffer;
			CreateBufferOnMFMediaBuffer(buffer.Get(), &rtBuffer);
			auto sample = MediaStreamSample::CreateFromBuffer(reinterpret_cast<IBuffer^>(rtBuffer.Get()), TimeSpan{ timestamp });
			sample->Duration = TimeSpan{ duration };
			sample->KeyFrame = true;
			request.Request->Sample = std::move(sample);
			request.Deferral->Complete();
			_sampleCache.pop();
			_requestCache.pop();
		}
		if (!_requestCache.empty())
		{
			if(_endOfStream.load(std::memory_order_acquire))
			{
				auto request = std::move(_requestCache.front());
				request.Deferral->Complete();
				_requestCache.pop();
			}
			else
				needMore = true;
		}
	}
	if (needMore)
		ThrowIfFailed(_sourceReader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr, nullptr, nullptr));
}

void EffectMediaStreamSource::ClearSampleAndRequestQueue()
{
	std::unique_lock<decltype(_requestCacheMutex)> requestLocker(_requestCacheMutex, std::defer_lock);
	std::unique_lock<decltype(_sampleCacheMutex)> sampleLocker(_sampleCacheMutex, std::defer_lock);
	std::lock(requestLocker, sampleLocker);

	_requestCache.swap(decltype(_requestCache)());
	_sampleCache.swap(decltype(_sampleCache)());
}

void EffectMediaStreamSource::OnStarting(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args)
{
	auto request = args->Request;
	MFTIME startPosition = 0;
	if (request->StartPosition)
		startPosition = request->StartPosition->Value.Duration;
	try
	{
		task_completion_event<void> flushOp;
		{
			std::lock_guard<decltype(_flushOperationsMutex)> locker(_flushOperationsMutex);
			_flushOperations.swap(decltype(_flushOperations)());
			_flushOperations.emplace(flushOp);
		}

		Platform::Agile<MediaStreamSourceStartingRequestDeferral> deferral(request->GetDeferral());
		create_task(flushOp).then([this, startPosition, request]
		{
			PROPVARIANT propStartPosition;
			propStartPosition.vt = VT_I8;
			propStartPosition.hVal.QuadPart = startPosition;

			ThrowIfFailed(_sourceReader->SetCurrentPosition(GUID_NULL, propStartPosition));
			request->SetActualStartPosition(TimeSpan{ startPosition });
			ClearSampleAndRequestQueue();
			_endOfStream.store(false, std::memory_order_release);
		}).then([this, deferral](task<void> t)
		{
			try
			{
				t.get();
			}
			catch (...)
			{
				_mss->NotifyError(MediaStreamSourceErrorStatus::FailedToOpenFile);
			}
			deferral->Complete();
		});
		ThrowIfFailed(_sourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS));
	}
	catch (...)
	{
		_mss->NotifyError(MediaStreamSourceErrorStatus::FailedToOpenFile);
		request->GetDeferral()->Complete();
	}
}

void EffectMediaStreamSource::OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args)
{
	{
		std::lock_guard<decltype(_requestCacheMutex)> locker(_requestCacheMutex);
		_requestCache.emplace(args->Request, args->Request->GetDeferral());
	}
	DispatchSampleRequests();
}


void EffectMediaStreamSource::OnClosed(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args)
{

}
