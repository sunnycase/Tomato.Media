//
// Tomato Media
// Media Engine
// 作者：SunnyCase
// 创建时间：2015-08-06
//
#include "pch.h"
#include "MediaEngine.h"
#include "MediaRenderSink.h"
#include "PresentationClock.h"
#include "../../include/MFAsyncCallback.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

std::shared_ptr<MediaEngine> MediaEngine::MakeMediaEngine()
{
	return std::shared_ptr<MediaEngine>(new MediaEngine());
}

MediaEngine::MediaEngine()
	:sourceReader(Make<VideoSourceReader>()), mediaSink(Make<MediaRenderSink>()), presentationClock(CreateHighResolutionPresentationClock())
{
	ThrowIfFailed(mediaSink->SetPresentationClock(presentationClock.Get()));
}

void MediaEngine::SetMediaSource(IMFMediaSource* mediaSource)
{
	ConfigureSourceReader(mediaSource);
}

void MediaEngine::ConfigureSourceReader(IMFMediaSource* mediaSource)
{
	sourceReader->SetMediaSource(mediaSource);
	ConfigureMediaSink();
}

void MediaEngine::ConfigureMediaSink()
{
	ThrowIfFailed(mediaSink->GetStreamSinkByIndex(1, &videoSink));
	ComPtr<IMFMediaTypeHandler> mediaTypeHandler;
	ThrowIfFailed(videoSink->GetMediaTypeHandler(&mediaTypeHandler));
	ThrowIfFailed(mediaTypeHandler->SetCurrentMediaType(sourceReader->OutputMediaType));
	
	videoSinkEventCallback = Make<MFAsyncCallback<MediaEngine>>(shared_from_this(), &MediaEngine::OnVideoSinkEvent);
	ThrowIfFailed(videoSink->BeginGetEvent(videoSinkEventCallback.Get(), nullptr));
}

HRESULT MediaEngine::OnVideoSinkEvent(IMFAsyncResult * pAsyncResult)
{
	try
	{
		ComPtr<IMFMediaEvent> event;
		MediaEventType eventType;
		ThrowIfFailed(videoSink->EndGetEvent(pAsyncResult, &event));
		ThrowIfFailed(event->GetType(&eventType));

		switch (eventType)
		{
		case MEStreamSinkRequestSample:
			OnVideoStreamSinkRequestSample();
			break;
		case MEStreamSinkPrerolled:
			OnVideoStreamSinkPrerolled();
			break;
		default:
			break;
		}

		ThrowIfFailed(videoSink->BeginGetEvent(videoSinkEventCallback.Get(), nullptr));
	}
	catch (...)
	{
		videoSink->BeginGetEvent(videoSinkEventCallback.Get(), nullptr);
		throw;
	}

	return S_OK;
}

void MediaEngine::OnVideoStreamSinkRequestSample()
{
	ComPtr<IMFSample> videoSample;
	if (sourceReader->TryReadVideoSample(videoSample))
		ThrowIfFailed(videoSink->ProcessSample(videoSample.Get()));
	else
	{
		std::weak_ptr<MediaEngine> weak(shared_from_this());
		sourceReader->ReadVideoSampleAsync().then([weak](ComPtr<IMFSample>& videoSample)
		{
			if (auto me = weak.lock())
				ThrowIfFailed(me->videoSink->ProcessSample(videoSample.Get()));
		}).then([](task<void> t) -> HRESULT
		{
			// 发生异常则忽略该次请求
			try
			{
				t.get();
				return S_OK;
			}
			CATCH_ALL();
		});
	}
}

void MediaEngine::OnVideoStreamSinkPrerolled()
{
	presentationClock->Start(PRESENTATION_CURRENT_POSITION);
}

void MediaEngine::Play()
{
	sourceReader->Start();

	// 检测是否支持缓冲
	ComPtr<IMFMediaSinkPreroll> prerollSink;
	if (SUCCEEDED(mediaSink.As(&prerollSink)))
	{
		ThrowIfFailed(prerollSink->NotifyPreroll(PRESENTATION_CURRENT_POSITION));
	}
	else
	{
		presentationClock->Start(PRESENTATION_CURRENT_POSITION);
	}
}

ComPtr<IVideoRender> MediaEngine::get_VideoRender() const
{
	ComPtr<IVideoRender> videoRender;
	ThrowIfFailed(MFGetService(mediaSink.Get(), MF_TM_VIDEORENDER_SERVICE, IID_PPV_ARGS(&videoRender)));
	return videoRender;
}