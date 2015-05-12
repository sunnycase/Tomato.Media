//
// Tomato Media
// Media Foundation Audio Source 基类
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#include "pch.h"
#include "AudioSourceBase.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> locker(stateMutex)

AudioSourceBase::AudioSourceBase()
	:MFOperationQueue(WorkerQueueProvider::GetProAudio())
{

}

AudioSourceBase::~AudioSourceBase()
{

}

//-------------------------------------------------------------------
// IMFMediaEventGenerator methods
//
// All of the IMFMediaEventGenerator methods do the following:
// 1. Check for shutdown status.
// 2. Call the event queue helper object.
//-------------------------------------------------------------------

HRESULT AudioSourceBase::BeginGetEvent(IMFAsyncCallback *pCallback, IUnknown *punkState)
{
	LOCK_STATE();
	if (!HasShutdown())
		return eventQueue->BeginGetEvent(pCallback, punkState);
	return MF_E_SHUTDOWN;
}

HRESULT AudioSourceBase::EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent)
{
	LOCK_STATE();
	if (!HasShutdown())
		return eventQueue->EndGetEvent(pResult, ppEvent);
	return MF_E_SHUTDOWN;
}

HRESULT AudioSourceBase::GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent)
{
	// NOTE:
	// GetEvent can block indefinitely, so we don't hold the lock.
	// This requires some juggling with the event queue pointer.
	ComPtr<IMFMediaEventQueue> spQueue;
	{
		LOCK_STATE();
		if (!HasShutdown())
			spQueue = eventQueue;
	}
	if (spQueue)
		return spQueue->GetEvent(dwFlags, ppEvent);
	return MF_E_SHUTDOWN;
}

HRESULT AudioSourceBase::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue)
{
	LOCK_STATE();
	if (!HasShutdown())
		return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
	return MF_E_SHUTDOWN;
}

//-------------------------------------------------------------------
// IMFMediaSource methods

//-------------------------------------------------------------------
// CreatePresentationDescriptor
// 返回 presentation descriptor 的浅拷贝
//-------------------------------------------------------------------

HRESULT AudioSourceBase::CreatePresentationDescriptor(IMFPresentationDescriptor **ppPresentationDescriptor)
{
	if (!ppPresentationDescriptor)
		return E_POINTER;

	LOCK_STATE();
	// 检测是否已加载 PresentationDescriptor
	if (!presentDescriptor)
		return MF_E_NOT_INITIALIZED;
	//返回 presentation descriptor 的浅拷贝
	return presentDescriptor->Clone(ppPresentationDescriptor);
}

//-------------------------------------------------------------------
// GetCharacteristics
// Returns capabilities flags.
//-------------------------------------------------------------------

HRESULT AudioSourceBase::GetCharacteristics(DWORD *pdwCharacteristics)
{
	if (!pdwCharacteristics)
		return E_POINTER;

	try
	{
		*pdwCharacteristics = OnGetCharacteristics();
	}
	CATCH_ALL();
	return S_OK;
}

//-------------------------------------------------------------------
// Pause
// Pauses the source.
//-------------------------------------------------------------------

HRESULT AudioSourceBase::Pause()
{
	if (!HasShutdown())
	{
		QueueAsyncOperation(std::make_shared<MediaSourceOperation>(MediaSourceOperationKind::Pause));
		return S_OK;
	}
	return MF_E_SHUTDOWN;
}

//-------------------------------------------------------------------
// Shutdown
// Shuts down the source and releases all resources.
//-------------------------------------------------------------------

HRESULT AudioSourceBase::Shutdown()
{
	if (!HasShutdown())
	{
		OnShutdown();
		return S_OK;
	}
	return MF_E_SHUTDOWN;
}

//-------------------------------------------------------------------
// ValidatePresentationDescriptor:
// Validates the presentation descriptor that the caller specifies
// in IMFMediaSource::Start().
//
// Note: This method performs a basic sanity check on the PD. It is
// not intended to be a thorough validation.
//-------------------------------------------------------------------

bool AudioSourceBase::HasShutdown() const
{
	return state == MFMediaSourceState::NotInitialized ||
		state == MFMediaSourceState::Initializing;
}

void AudioSourceBase::ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
	BOOL hasSelectedStream = FALSE;
	DWORD streamCount = 0;

	if (!pPD) THROW_IF_FAILED(MF_E_INVALID_STREAM_STATE);

	OnValidatePresentationDescriptor(pPD);

	THROW_IF_FAILED(pPD->GetStreamDescriptorCount(&streamCount));

	// The caller must select at least one stream.
	for (DWORD i = 0; i < streamCount; i++)
	{
		ComPtr<IMFStreamDescriptor> spSD;
		THROW_IF_FAILED(pPD->GetStreamDescriptorByIndex(i, &hasSelectedStream, &spSD));
		if (hasSelectedStream)break;
	}

	if (!hasSelectedStream) THROW_IF_FAILED(MF_E_MEDIA_SOURCE_NO_STREAMS_SELECTED);
}

//-------------------------------------------------------------------
// Start
// Starts or seeks the media source.
//-------------------------------------------------------------------

HRESULT AudioSourceBase::Start(
	IMFPresentationDescriptor *pPresentationDescriptor,
	const GUID *pguidTimeFormat,
	const PROPVARIANT *pvarStartPos)
{
	// Check parameters.

	// Start position and presentation descriptor cannot be nullptr.
	if (pvarStartPos == nullptr || pPresentationDescriptor == nullptr)
		return E_INVALIDARG;

	// Check the time format.
	if ((pguidTimeFormat != nullptr) && (*pguidTimeFormat != GUID_NULL))
		// Unrecognized time format GUID.
		return MF_E_UNSUPPORTED_TIME_FORMAT;

	// Check the data type of the start position.
	if ((pvarStartPos->vt != VT_I8) && (pvarStartPos->vt != VT_EMPTY))
		return MF_E_UNSUPPORTED_TIME_FORMAT;
	REFERENCE_TIME position = pvarStartPos->vt == VT_EMPTY ? -1 : pvarStartPos->hVal.QuadPart;

	try
	{
		LOCK_STATE();
		ValidateStartOperation(state, position);
		// Perform a sanity check on the caller's presentation descriptor.
		ValidatePresentationDescriptor(pPresentationDescriptor);

		// The operation looks OK. Complete the operation asynchronously.
		auto startOperation = std::make_shared<MediaSourceStartOperation>(
			pPresentationDescriptor, position);
		QueueAsyncOperation(std::move(startOperation));
	}
	CATCH_ALL();
	return S_OK;
}

//-------------------------------------------------------------------
// Stop
// Stops the media source.
//-------------------------------------------------------------------

HRESULT AudioSourceBase::Stop()
{
	if (!HasShutdown())
	{
		QueueAsyncOperation(std::make_shared<MediaSourceOperation>(MediaSourceOperationKind::Stop));
		return S_OK;
	}
	return MF_E_SHUTDOWN;
}

//-------------------------------------------------------------------
// IMFGetService methods
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// GetService
// 返回一种支持的服务
// 支持服务有： RateControl
//-------------------------------------------------------------------

HRESULT AudioSourceBase::GetService(_In_ REFGUID guidService, _In_ REFIID riid, _Out_opt_ LPVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	// 不支持的服务
	return MF_E_UNSUPPORTED_SERVICE;
}

//-------------------------------------------------------------------
// ValidateOperation
//
// Checks whether the source can perform the operation indicated
// by pOp at this time.
//
// If the source cannot perform the operation now, the method
// returns MF_E_NOTACCEPTING.
//
// NOTE:
// Implements the pure-virtual OpQueue::ValidateOperation method.
//-------------------------------------------------------------------

void AudioSourceBase::ValidateOperation(TOperation& op)
{
	if (!op) THROW_IF_FAILED(MF_E_NOTACCEPTING);
}

//-------------------------------------------------------------------
// DispatchOperation
//
// Performs the asynchronous operation indicated by pOp.
//
// NOTE:
// This method implements the pure-virtual OpQueue::DispatchOperation
// method. It is always called from a work-queue thread.
//-------------------------------------------------------------------

void AudioSourceBase::DispatchOperation(TOperation& op)
{
	HRESULT hr = S_OK;

	try
	{
		LOCK_STATE();
		switch (op->GetKind())
		{
			// IMFMediaSource methods:
		case MediaSourceOperationKind::Start:
			DoStart(reinterpret_cast<MediaSourceStartOperation*>(op.get()));
			break;

		case MediaSourceOperationKind::Pause:
			DoPause();
			break;

		case MediaSourceOperationKind::Stop:
			DoStop();
			break;

			//	// Operations requested by the streams:
		case MediaSourceOperationKind::RequestData:
			OnStreamsRequestData(op).wait();
			break;

		case MediaSourceOperationKind::EndOfStream:
			OnEndOfStream();
			break;

		default:
			THROW_IF_FAILED(E_UNEXPECTED);
		}
	}
	catch (Platform::Exception^ ex)
	{
		hr = ex->HResult;
	}
	catch (...)
	{
		hr = E_FAIL;
	}
	if (FAILED(hr))
		QueueEvent(MEError, GUID_NULL, hr, nullptr);
}

task<void> AudioSourceBase::OpenAsync(IMFByteStream* byteStream)
{
	if (!byteStream) THROW_IF_FAILED(MF_E_INVALID_STREAM_DATA);

	{
		LOCK_STATE();
		if (state != MFMediaSourceState::NotInitialized)
			THROW_IF_FAILED(MF_E_INVALIDREQUEST);
		state = MFMediaSourceState::Initializing;
	}

	// 创建事件队列
	THROW_IF_FAILED(MFCreateEventQueue(eventQueue.ReleaseAndGetAddressOf()));

	DWORD Capabilities;
	THROW_IF_FAILED(byteStream->GetCapabilities(&Capabilities));
	// 检测流功能
	if (!(Capabilities & MFBYTESTREAM_IS_READABLE))
		THROW_IF_FAILED(MF_E_UNSUPPORTED_BYTESTREAM_TYPE);

	QWORD new_pos;
	THROW_IF_FAILED(byteStream->Seek(msoBegin, 0, MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &new_pos));
	return InitializeAudioSource(byteStream);
}

task<void> AudioSourceBase::InitializeAudioSource(ComPtr<IMFByteStream> stream)
{
	return CreatePresentationDescriptor(stream);
}

void AudioSourceBase::ValidateStartOperation(MFMediaSourceState state, REFERENCE_TIME position)
{

}

task<void> AudioSourceBase::CreatePresentationDescriptor(ComPtr<IMFByteStream> stream)
{
	{
		LOCK_STATE();
		if (state != MFMediaSourceState::Initializing)
			THROW_IF_FAILED(MF_E_MEDIA_SOURCE_WRONGSTATE);
	}

	return OnCreatePresentationDescriptor(stream)
		.then([this](ComPtr<IMFPresentationDescriptor> pd)
	{
		LOCK_STATE();
		this->presentDescriptor = pd;
		state = MFMediaSourceState::Starting;
	});
}

void AudioSourceBase::QueueAsyncOperation(TOperation&& operation)
{
	Enqueue(std::move(operation));
}

void AudioSourceBase::QueueAsyncOperation(MediaSourceOperationKind operation)
{
	QueueAsyncOperation(std::make_shared<MediaSourceOperation>(operation));
}

void AudioSourceBase::OnShutdown()
{

}

void AudioSourceBase::DoStart(MediaSourceStartOperation* operation)
{
	auto pd = operation->GetPresentationDescriptor();
	auto position = operation->GetPosition();

	// 启动流
	StartStreams(pd, position);

	{
		PROPVARIANT positionVar;
		PropVariantInit(&positionVar);
		if (position == -1)
			positionVar.vt = VT_EMPTY;
		else
		{
			positionVar.vt = VT_I8;
			positionVar.hVal.QuadPart = position;
		}

		LOCK_STATE();
		if (state == MFMediaSourceState::Starting)
		{
			state = MFMediaSourceState::Started;
			// Queue the "started" event. The event data is the start position.
			THROW_IF_FAILED(QueueEvent(MESourceStarted, GUID_NULL, S_OK, &positionVar));
		}
		else
		{
			state = MFMediaSourceState::Started;
			THROW_IF_FAILED(QueueEvent(MESourceSeeked, GUID_NULL, S_OK, &positionVar));
		}
	}
}

void AudioSourceBase::DoPause()
{
	LOCK_STATE();
	if (state == MFMediaSourceState::Started)
	{
		PauseStreams();

		state = MFMediaSourceState::Paused;
		THROW_IF_FAILED(QueueEvent(MESourcePaused, GUID_NULL, S_OK, nullptr));
	}
}

void AudioSourceBase::PauseStreams()
{
	BOOL selected = FALSE;
	DWORD streamId = 0;
	DWORD streamsCount = 0;

	// Reset the pending EOS count.
	pendingEOSCount = 0;

	THROW_IF_FAILED(presentDescriptor->GetStreamDescriptorCount(&streamsCount));
	for (size_t i = 0; i < streamsCount; i++)
	{
		ComPtr<IMFStreamDescriptor> streamDesc;

		THROW_IF_FAILED(presentDescriptor->GetStreamDescriptorByIndex(0, &selected, &streamDesc));
		if (selected)
			OnPauseStream(i);
	}
}

void AudioSourceBase::DoStop()
{
	LOCK_STATE();
	if (state == MFMediaSourceState::Paused || state == MFMediaSourceState::Started)
	{
		StopStreams();

		state = MFMediaSourceState::Stopped;
		THROW_IF_FAILED(QueueEvent(MESourceStopped, GUID_NULL, S_OK, nullptr));
	}
}

void AudioSourceBase::StopStreams()
{
	BOOL selected = FALSE;
	DWORD streamId = 0;
	DWORD streamsCount = 0;

	// Reset the pending EOS count.
	pendingEOSCount = 0;

	THROW_IF_FAILED(presentDescriptor->GetStreamDescriptorCount(&streamsCount));
	for (size_t i = 0; i < streamsCount; i++)
	{
		ComPtr<IMFStreamDescriptor> streamDesc;

		THROW_IF_FAILED(presentDescriptor->GetStreamDescriptorByIndex(0, &selected, &streamDesc));
		if (selected)
			OnStopStream(i);
	}
}

HRESULT AudioSourceBase::QueueEventUnk(MediaEventType met, REFGUID guidExtendedType,
	HRESULT hrStatus, IUnknown *unk)
{
	LOCK_STATE();
	if (!HasShutdown())
		return eventQueue->QueueEventParamUnk(met, guidExtendedType, hrStatus, unk);
	return MF_E_SHUTDOWN;
}

void AudioSourceBase::OnEndOfStream()
{
	if (!--pendingEOSCount)
	{
		// No more streams. Send the end-of-presentation event.
		THROW_IF_FAILED(QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, nullptr));
	}
}

void AudioSourceBase::StartStreams(IMFPresentationDescriptor* pd, REFERENCE_TIME position)
{
	BOOL selected = FALSE;
	DWORD streamId = 0;
	DWORD streamsCount = 0;

	// Reset the pending EOS count.
	pendingEOSCount = 0;

	THROW_IF_FAILED(pd->GetStreamDescriptorCount(&streamsCount));

	for (size_t i = 0; i < streamsCount; i++)
	{
		// Loop throught the stream descriptors to find which streams are active.
		ComPtr<IMFStreamDescriptor> streamDesc;

		THROW_IF_FAILED(pd->GetStreamDescriptorByIndex(0, &selected, &streamDesc));
		THROW_IF_FAILED(streamDesc->GetStreamIdentifier(&streamId));

		OnStartStream(streamId, selected == TRUE, position);
		if (selected) pendingEOSCount++;
	}
}