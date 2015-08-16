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
	:StreamRenderSinkBase(identifier, mediaSink), videoRender(videoRender)
{
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
		videoRender->RenderFrame(frame);
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
	ThrowIfFailed(eventQueue->QueueEventParamVar(MEStreamSinkRequestSample, GUID_NULL, S_OK, nullptr));
}

void VideoStreamRenderSink::PostSampleRequestIfNeeded()
{
}
