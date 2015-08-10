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

VideoStreamRenderSink::VideoStreamRenderSink(DWORD identifier, IMFMediaSink* mediaSink, IVideoRender* videoRender)
	:identifier(identifier), mediaSink(mediaSink), videoRender(videoRender)
{
	ThrowIfFailed(MFCreateEventQueue(&eventQueue));
}

HRESULT VideoStreamRenderSink::GetEvent(DWORD dwFlags, IMFMediaEvent ** ppEvent)
{
	return eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT VideoStreamRenderSink::BeginGetEvent(IMFAsyncCallback * pCallback, IUnknown * punkState)
{
	return eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT VideoStreamRenderSink::EndGetEvent(IMFAsyncResult * pResult, IMFMediaEvent ** ppEvent)
{
	return eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT VideoStreamRenderSink::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT * pvValue)
{
	return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

HRESULT VideoStreamRenderSink::GetMediaSink(IMFMediaSink ** ppMediaSink)
{
	*ppMediaSink = mediaSink;
	mediaSink->AddRef();
	return S_OK;
}

HRESULT VideoStreamRenderSink::GetIdentifier(DWORD * pdwIdentifier)
{
	*pdwIdentifier = identifier;
	return S_OK;
}

HRESULT VideoStreamRenderSink::GetMediaTypeHandler(IMFMediaTypeHandler ** ppHandler)
{
	*ppHandler = this;
	return S_OK;
}

HRESULT VideoStreamRenderSink::ProcessSample(IMFSample * pSample)
{
	try
	{
		auto frame = videoRender->CreateFrame(pSample, frameWidth, frameHeight);
		videoRender->RenderFrame(frame.Get());
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
	auto hr = IsMediaTypeSupported(pMediaType, nullptr);
	if (SUCCEEDED(hr))
	{
		try
		{
			std::lock_guard<decltype(stateMutex)> locker(stateMutex);
			if (mediaType.Get() != pMediaType)
			{
				mediaType = pMediaType;
				OnSetMediaType();
			}
		}
		CATCH_ALL();
	}
	return hr;
}

HRESULT VideoStreamRenderSink::GetCurrentMediaType(IMFMediaType ** ppMediaType)
{
	try
	{
		std::lock_guard<decltype(stateMutex)> locker(stateMutex);
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

void VideoStreamRenderSink::OnSetMediaType()
{
	videoRender->Initialize();
	ThrowIfFailed(MFGetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, &frameWidth, &frameHeight));
}