//
// Tomato Media
// Media Foundation 音频流
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#include "pch.h"
#include "MFAudioStream.h"
#include <sstream>

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

MFAudioStream::MFAudioStream(AudioSourceBase* audioSource, ComPtr<IMFStreamDescriptor> streamDescriptor)
	:audioSource(audioSource),
	streamDescriptor(streamDescriptor),
	endOfDeliver(true)
{
	THROW_IF_FAILED(MFCreateEventQueue(&eventQueue));
	state = MFMediaStreamState::Opening;
}

MFAudioStream::~MFAudioStream()
{

}

// ------------------------------------------------------------------ -
// IMFMediaEventGenerator methods
//
// For remarks, see MPEG1Source.cpp
//-------------------------------------------------------------------

HRESULT MFAudioStream::BeginGetEvent(IMFAsyncCallback *pCallback, IUnknown *punkState)
{
	return eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT MFAudioStream::EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent)
{
	return eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT MFAudioStream::GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent)
{
	return eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT MFAudioStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue)
{
	return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

//-------------------------------------------------------------------
// IMFMediaStream methods
//-------------------------------------------------------------------
// GetMediaSource:
// Returns a pointer to the media source.
//-------------------------------------------------------------------

HRESULT MFAudioStream::GetMediaSource(IMFMediaSource **ppMediaSource)
{
	if (!ppMediaSource) return E_POINTER;

	if (!this->audioSource) return E_UNEXPECTED;

	*ppMediaSource = audioSource;
	return S_OK;
}

//-------------------------------------------------------------------
// GetStreamDescriptor:
// Returns a pointer to the stream descriptor for this stream.
//-------------------------------------------------------------------

HRESULT MFAudioStream::GetStreamDescriptor(IMFStreamDescriptor **ppStreamDescriptor)
{
	if (!ppStreamDescriptor) return E_POINTER;

	if (!this->streamDescriptor) return E_UNEXPECTED;

	this->streamDescriptor.CopyTo(ppStreamDescriptor);
	return S_OK;
}

//-------------------------------------------------------------------
// RequestSample:
// Requests data from the stream.
//
// pToken: Token used to track the request. Can be nullptr.
//-------------------------------------------------------------------

HRESULT MFAudioStream::RequestSample(IUnknown *pToken)
{
	std::lock_guard<decltype(sampleMutex)> locker(sampleMutex);
	HRESULT hr = S_OK;
	try
	{
		if (state == MFMediaStreamState::Stopped)
			THROW_IF_FAILED(MF_E_INVALIDREQUEST)
		else if (state == MFMediaStreamState::EndOfStream && samplesCache.empty())
		THROW_IF_FAILED(MF_E_END_OF_STREAM);

		sampleRequests.emplace(pToken);
		DispatchSampleRequests();
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
		audioSource->QueueEvent(MEError, GUID_NULL, hr, nullptr);
	return hr;
}

void MFAudioStream::DispatchSampleRequests()
{
	std::lock_guard<decltype(sampleMutex)> locker(sampleMutex);
	if (state != MFMediaStreamState::Started)
		return;

	while (!samplesCache.empty() && !sampleRequests.empty())
	{
		auto sample = samplesCache.front();
		samplesCache.pop();

		auto token = sampleRequests.front();
		sampleRequests.pop();
		if (token)
			sample->SetUnknown(MFSampleExtension_Token, token.Get());
		THROW_IF_FAILED(eventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, S_OK, sample.Get()));
	}
	if (samplesCache.empty() && endOfDeliver)
		OnEndOfStream();

	if (DoesNeedMoreData())
		audioSource->QueueAsyncOperation(MediaSourceOperationKind::RequestData);
}

void MFAudioStream::OnEndOfStream()
{
	if (state != MFMediaStreamState::NotInitialized)
	{
		state = MFMediaStreamState::EndOfStream;
		THROW_IF_FAILED(eventQueue->QueueEventParamUnk(MEEndOfStream, GUID_NULL, S_OK, nullptr));
		// 提示音源本流结束
		audioSource->QueueAsyncOperation(MediaSourceOperationKind::EndOfStream);
	}
}

void MFAudioStream::EndOfDeliver()
{
	endOfDeliver = true;
}

bool MFAudioStream::DoesNeedMoreData()
{
	std::wstringstream ss;
	ss << L"samples cache: " << samplesCache.size() << std::endl;
	OutputDebugString(ss.str().c_str());

	return !endOfDeliver && state == MFMediaStreamState::Started && samplesCache.size() < PreRollSample;
}

void MFAudioStream::DeliverPayload(IMFSample* sample)
{
	if (!sample) THROW_IF_FAILED(E_INVALIDARG);

	samplesCache.emplace(sample);
	DispatchSampleRequests();
}

MFMediaStreamState MFAudioStream::GetState() const noexcept
{
	return state;
}

void MFAudioStream::Start(REFERENCE_TIME position)
{
	endOfDeliver = false;

	if (state != MFMediaStreamState::NotInitialized)
	{
		state = MFMediaStreamState::Started;

		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_I8;
		varStart.hVal.QuadPart = position;
		THROW_IF_FAILED(QueueEvent(MEStreamStarted, GUID_NULL, S_OK, &varStart));
		DispatchSampleRequests();
	}
}

void MFAudioStream::Pause()
{
	if (state == MFMediaStreamState::Started)
		state = MFMediaStreamState::Paused;
}