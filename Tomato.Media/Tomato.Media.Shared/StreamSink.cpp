//
// Tomato Media
// Stream Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#include "pch.h"
#include "StreamSink.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

StreamSink::StreamSink(DWORD identifier, IMFMediaSink* mediaSink)
	:identifier(identifier), mediaSink(mediaSink)
{
	ThrowIfFailed(MFCreateEventQueue(&eventQueue));
}

HRESULT StreamSink::GetEvent(DWORD dwFlags, IMFMediaEvent ** ppEvent)
{
	return eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT StreamSink::BeginGetEvent(IMFAsyncCallback * pCallback, IUnknown * punkState)
{
	return eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT StreamSink::EndGetEvent(IMFAsyncResult * pResult, IMFMediaEvent ** ppEvent)
{
	return eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT StreamSink::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT * pvValue)
{
	return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

HRESULT StreamSink::GetMediaSink(IMFMediaSink ** ppMediaSink)
{
	*ppMediaSink = mediaSink;
	mediaSink->AddRef();
	return S_OK;
}

HRESULT StreamSink::GetIdentifier(DWORD * pdwIdentifier)
{
	*pdwIdentifier = identifier;
	return S_OK;
}

HRESULT StreamSink::GetMediaTypeHandler(IMFMediaTypeHandler ** ppHandler)
{
	*ppHandler = this;
	return S_OK;
}

HRESULT StreamSink::ProcessSample(IMFSample * pSample)
{
	return S_OK;
}

HRESULT StreamSink::PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT * pvarMarkerValue, const PROPVARIANT * pvarContextValue)
{
	return S_OK;
}

HRESULT StreamSink::Flush(void)
{
	return S_OK;
}

HRESULT StreamSink::IsMediaTypeSupported(IMFMediaType * pMediaType, IMFMediaType ** ppMediaType)
{
	try
	{
		GUID majorType;
		ThrowIfFailed(pMediaType->GetMajorType(&majorType));

		const static std::vector<GUID> audioSubTypes = {
			MFAudioFormat_Float,
			MFAudioFormat_PCM
		};

		const static std::vector<GUID> videoSubTypes = {
			MFVideoFormat_NV12
		};

		const std::vector<GUID>* supportedSubTypes = nullptr;

		if (majorType == MFMediaType_Audio)
			supportedSubTypes = &audioSubTypes;
		else if(majorType == MFMediaType_Video)
			supportedSubTypes = &videoSubTypes;

		if (supportedSubTypes)
		{
			GUID subType;
			ThrowIfFailed(pMediaType->GetGUID(MF_MT_SUBTYPE, &subType));
			auto it = std::find(supportedSubTypes->begin(), supportedSubTypes->end(), subType);
			if (it != supportedSubTypes->end())
				return S_OK;
			else if (ppMediaType)
			{
				ComPtr<IMFMediaType> desiredMediaType;
				ThrowIfFailed(MFCreateMediaType(&desiredMediaType));
				ThrowIfFailed(desiredMediaType->SetGUID(MF_MT_MAJOR_TYPE, majorType));
				ThrowIfFailed(desiredMediaType->SetGUID(MF_MT_SUBTYPE, supportedSubTypes->front()));
				ThrowIfFailed(desiredMediaType.CopyTo(ppMediaType));
			}
		}
		return MF_E_INVALIDMEDIATYPE;
	}
	CATCH_ALL();

	return S_OK;
}

HRESULT StreamSink::GetMediaTypeCount(DWORD * pdwTypeCount)
{
	return E_NOTIMPL;
}

HRESULT StreamSink::GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType ** ppType)
{
	return E_NOTIMPL;
}

HRESULT StreamSink::SetCurrentMediaType(IMFMediaType * pMediaType)
{
	auto hr = IsMediaTypeSupported(pMediaType, nullptr);
	if (SUCCEEDED(hr))
		mediaType = pMediaType;
	return hr;
}

HRESULT StreamSink::GetCurrentMediaType(IMFMediaType ** ppMediaType)
{
	if (mediaType)
		return mediaType.CopyTo(ppMediaType);
	return MF_E_NOT_INITIALIZED;
}

HRESULT StreamSink::GetMajorType(GUID * pguidMajorType)
{
	if (mediaType)
		return mediaType->GetMajorType(pguidMajorType);
	return MF_E_NOT_INITIALIZED;
}
