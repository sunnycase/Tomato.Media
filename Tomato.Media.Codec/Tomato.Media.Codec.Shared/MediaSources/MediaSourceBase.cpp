//
// Tomato Media Codec
// Media Foundation MediaSource 基类
// 
// 作者：SunnyCase
// 创建时间：2015-03-17
#include "pch.h"
#include "MediaSourceBase.h"
#include "../../include/MFWorkerQueueProvider.h"
#include <libavcodec/avcodec.h>

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> locker(stateMutex)

MediaSourceBase::MediaSourceBase()
	:operationQueue(std::make_shared<Core::MFOperationQueue<TOperation>>(
		[weak = AsWeak()](TOperation& op)
{
	if (auto me = weak.Resolve<MediaSourceBase>()) me->OnDispatchOperation(op);
}))
{
	operationQueue->SetWorkerQueue(Core::MFWorkerQueueProvider::GetAudio());
}

MediaSourceBase::~MediaSourceBase()
{

}

//-------------------------------------------------------------------
// IMFMediaEventGenerator methods
//
// All of the IMFMediaEventGenerator methods do the following:
// 1. Check for shutdown status.
// 2. Call the event queue helper object.
//-------------------------------------------------------------------

HRESULT MediaSourceBase::BeginGetEvent(IMFAsyncCallback *pCallback, IUnknown *punkState)
{
	return eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT MediaSourceBase::EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent)
{
	return eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT MediaSourceBase::GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent)
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
	return spQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT MediaSourceBase::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue)
{
	return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

//-------------------------------------------------------------------
// IMFMediaSource methods

//-------------------------------------------------------------------
// CreatePresentationDescriptor
// 返回 presentation descriptor 的浅拷贝
//-------------------------------------------------------------------

HRESULT MediaSourceBase::CreatePresentationDescriptor(IMFPresentationDescriptor **ppPresentationDescriptor)
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

HRESULT MediaSourceBase::GetCharacteristics(DWORD *pdwCharacteristics)
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

HRESULT MediaSourceBase::Pause()
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

HRESULT MediaSourceBase::Shutdown()
{
	if (!HasShutdown())
	{
		OnShutdown();
		state = MFMediaSourceState::NotInitialized;
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

bool MediaSourceBase::HasShutdown() const
{
	return state == MFMediaSourceState::NotInitialized ||
		state == MFMediaSourceState::Initializing;
}

void MediaSourceBase::ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
	BOOL hasSelectedStream = FALSE;
	DWORD streamCount = 0;

	if (!pPD) ThrowIfFailed(MF_E_INVALID_STREAM_STATE);

	OnValidatePresentationDescriptor(pPD);

	ThrowIfFailed(pPD->GetStreamDescriptorCount(&streamCount));

	// The caller must select at least one stream.
	for (DWORD i = 0; i < streamCount; i++)
	{
		ComPtr<IMFStreamDescriptor> spSD;
		ThrowIfFailed(pPD->GetStreamDescriptorByIndex(i, &hasSelectedStream, &spSD));
		if (hasSelectedStream)break;
	}

	if (!hasSelectedStream) ThrowIfFailed(MF_E_MEDIA_SOURCE_NO_STREAMS_SELECTED);
}

//-------------------------------------------------------------------
// Start
// Starts or seeks the media source.
//-------------------------------------------------------------------

HRESULT MediaSourceBase::Start(
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
		state = MFMediaSourceState::Starting;
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

HRESULT MediaSourceBase::Stop()
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

HRESULT MediaSourceBase::GetService(_In_ REFGUID guidService, _In_ REFIID riid, _Out_opt_ LPVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	//if (guidService == MF_RATE_CONTROL_SERVICE)
	//	return this->QueryInterface(riid, ppvObject);

	// 不支持的服务
	return MF_E_UNSUPPORTED_SERVICE;
}

STDMETHODIMP MediaSourceBase::GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float * pflRate)
{
	if (eDirection != MFRATE_FORWARD) return MF_E_UNSUPPORTED_RATE;
	if (fThin) return MF_E_THINNING_UNSUPPORTED;
	*pflRate = 1.0f;
	return S_OK;
}

STDMETHODIMP MediaSourceBase::GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float * pflRate)
{
	if (eDirection != MFRATE_FORWARD) return MF_E_UNSUPPORTED_RATE;
	if (fThin) return MF_E_THINNING_UNSUPPORTED;
	*pflRate = 1.0f;
	return S_OK;
}

STDMETHODIMP MediaSourceBase::IsRateSupported(BOOL fThin, float flRate, float * pflNearestSupportedRate)
{
	if (fThin) return MF_E_THINNING_UNSUPPORTED;
	if (flRate != 1.0f)return MF_E_UNSUPPORTED_RATE;
	return S_OK;
}

STDMETHODIMP MediaSourceBase::SetRate(BOOL fThin, float flRate)
{
	if (fThin) return MF_E_THINNING_UNSUPPORTED;
	thin = fThin;
	rate = flRate;
	return S_OK;
}

STDMETHODIMP MediaSourceBase::GetRate(BOOL * pfThin, float * pflRate)
{
	*pfThin = thin;
	*pflRate = rate;
	return S_OK;
}

//-------------------------------------------------------------------
// DispatchOperation
//
// Checks whether the source can perform the operation indicated
// by pOp at this time.
//
// If the source cannot perform the operation now, the method
// returns MF_E_NOTACCEPTING.
//
// Performs the asynchronous operation indicated by pOp.
//
//-------------------------------------------------------------------

void MediaSourceBase::OnDispatchOperation(TOperation& op)
{
	if (!op) ThrowIfFailed(MF_E_NOTACCEPTING);

	HRESULT hr = S_OK;
	try
	{
		switch (op->GetKind())
		{
			// IMFMediaSource methods:
		case MediaSourceOperationKind::Start:
			DoStart(static_cast<MediaSourceStartOperation*>(op.get()));
			break;

		case MediaSourceOperationKind::Pause:
			DoPause();
			break;

		case MediaSourceOperationKind::Stop:
			DoStop();
			break;

			// Operations requested by the streams:
		case MediaSourceOperationKind::RequestData:
			OnStreamsRequestData(static_cast<MediaStreamRequestDataOperation*>(
				op.get())->GetMediaStream()).wait();
			break;

		case MediaSourceOperationKind::EndOfStream:
			OnEndOfStream();
			break;

		default:
			ThrowIfFailed(E_UNEXPECTED);
		}
	}
	CATCH_ALL_WITHHR(hr);

	if (FAILED(hr))
		QueueEvent(MEError, GUID_NULL, hr, nullptr);
}

task<void> MediaSourceBase::OpenAsync(IMFByteStream* byteStream)
{
	if (!byteStream) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);

	{
		LOCK_STATE();
		if (state != MFMediaSourceState::NotInitialized)
			ThrowIfFailed(MF_E_INVALIDREQUEST);
		state = MFMediaSourceState::Initializing;
	}

	// 创建事件队列
	ThrowIfFailed(MFCreateEventQueue(eventQueue.ReleaseAndGetAddressOf()));

	DWORD Capabilities;
	ThrowIfFailed(byteStream->GetCapabilities(&Capabilities));
	// 检测流功能
	if (!(Capabilities & MFBYTESTREAM_IS_READABLE))
		ThrowIfFailed(MF_E_UNSUPPORTED_BYTESTREAM_TYPE);

	QWORD new_pos;
	ThrowIfFailed(byteStream->Seek(msoBegin, 0, MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &new_pos));
	return InitializeAudioSource(byteStream);
}

task<void> MediaSourceBase::InitializeAudioSource(IMFByteStream* stream)
{
	return CreatePresentationDescriptor(stream);
}

void MediaSourceBase::ValidateStartOperation(MFMediaSourceState state, MFTIME position)
{

}

task<void> MediaSourceBase::CreatePresentationDescriptor(IMFByteStream* stream)
{
	{
		LOCK_STATE();
		if (state != MFMediaSourceState::Initializing)
			ThrowIfFailed(MF_E_MEDIA_SOURCE_WRONGSTATE);
	}

	return OnCreatePresentationDescriptor(stream)
		.then([this](ComPtr<IMFPresentationDescriptor> pd)
	{
		LOCK_STATE();
		this->presentDescriptor = pd;
		state = MFMediaSourceState::Stopped;
	});
}

void MediaSourceBase::QueueAsyncOperation(TOperation&& operation)
{
	operationQueue->Enqueue(std::move(operation));
}

void MediaSourceBase::QueueAsyncOperation(MediaSourceOperationKind operation)
{
	QueueAsyncOperation(std::make_shared<MediaSourceOperation>(operation));
}

void MediaSourceBase::OnShutdown()
{
	eventQueue->Shutdown();
	presentDescriptor = nullptr;
	operationQueue = nullptr;
}

void MediaSourceBase::DoStart(MediaSourceStartOperation* operation)
{
	auto pd = operation->GetPresentationDescriptor();
	auto position = operation->GetPosition();

	OnSeekSource(position);

	PROPVARIANT positionVar;
	PropVariantInit(&positionVar);
	auto fin = make_finalizer([&] {PropVariantClear(&positionVar); });

	if (position == -1)
		positionVar.vt = VT_EMPTY;
	else
	{
		positionVar.vt = VT_I8;
		positionVar.hVal.QuadPart = position;
	}
	// 启动流
	StartStreams(pd, positionVar);

	LOCK_STATE();
	if (state == MFMediaSourceState::Starting)
	{
		state = MFMediaSourceState::Started;
		// Queue the "started" event. The event data is the start position.
		ThrowIfFailed(eventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, S_OK, &positionVar));
	}
	else
	{
		state = MFMediaSourceState::Started;
		ThrowIfFailed(eventQueue->QueueEventParamVar(MESourceSeeked, GUID_NULL, S_OK, &positionVar));
	}
}

void MediaSourceBase::DoPause()
{
	LOCK_STATE();
	if (state == MFMediaSourceState::Started)
	{
		PauseStreams();

		state = MFMediaSourceState::Paused;
		ThrowIfFailed(eventQueue->QueueEventParamVar(MESourcePaused, GUID_NULL, S_OK, nullptr));
	}
}

void MediaSourceBase::PauseStreams()
{
	BOOL selected = FALSE;
	DWORD streamId = 0;
	DWORD streamsCount = 0;

	// Reset the pending EOS count.
	pendingEOSCount = 0;

	ThrowIfFailed(presentDescriptor->GetStreamDescriptorCount(&streamsCount));
	for (size_t i = 0; i < streamsCount; i++)
	{
		ComPtr<IMFStreamDescriptor> streamDesc;

		ThrowIfFailed(presentDescriptor->GetStreamDescriptorByIndex(0, &selected, &streamDesc));
		if (selected)
		{
			DWORD streamId;
			ThrowIfFailed(streamDesc->GetStreamIdentifier(&streamId));
			OnPauseStream(streamId);
		}
	}
}

void MediaSourceBase::DoStop()
{
	LOCK_STATE();
	if (state == MFMediaSourceState::Paused || state == MFMediaSourceState::Started)
	{
		StopStreams();

		state = MFMediaSourceState::Stopped;
		ThrowIfFailed(eventQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, S_OK, nullptr));
	}
}

void MediaSourceBase::StopStreams()
{
	BOOL selected = FALSE;
	DWORD streamId = 0;
	DWORD streamsCount = 0;

	// Reset the pending EOS count.
	pendingEOSCount = 0;

	ThrowIfFailed(presentDescriptor->GetStreamDescriptorCount(&streamsCount));
	for (size_t i = 0; i < streamsCount; i++)
	{
		ComPtr<IMFStreamDescriptor> streamDesc;

		ThrowIfFailed(presentDescriptor->GetStreamDescriptorByIndex(0, &selected, &streamDesc));
		if (selected)
		{
			DWORD streamId;
			ThrowIfFailed(streamDesc->GetStreamIdentifier(&streamId));
			OnStopStream(streamId);
		}
	}
}

HRESULT MediaSourceBase::QueueEventUnk(MediaEventType met, REFGUID guidExtendedType,
	HRESULT hrStatus, IUnknown *unk)
{
	return eventQueue->QueueEventParamUnk(met, guidExtendedType, hrStatus, unk);
}

void MediaSourceBase::OnEndOfStream()
{
	if (!--pendingEOSCount)
	{
		// No more streams. Send the end-of-presentation event.
		ThrowIfFailed(QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, nullptr));
	}
}

void MediaSourceBase::StartStreams(IMFPresentationDescriptor* pd, const PROPVARIANT& position)
{
	BOOL selected = FALSE;
	DWORD streamId = 0;
	DWORD streamsCount = 0;

	// Reset the pending EOS count.
	pendingEOSCount = 0;

	ThrowIfFailed(pd->GetStreamDescriptorCount(&streamsCount));

	for (size_t i = 0; i < streamsCount; i++)
	{
		// Loop throught the stream descriptors to find which streams are active.
		ComPtr<IMFStreamDescriptor> streamDesc;

		ThrowIfFailed(pd->GetStreamDescriptorByIndex(0, &selected, &streamDesc));
		ThrowIfFailed(streamDesc->GetStreamIdentifier(&streamId));

		OnStartStream(streamId, selected == TRUE, position);
		if (selected) pendingEOSCount++;
	}
}