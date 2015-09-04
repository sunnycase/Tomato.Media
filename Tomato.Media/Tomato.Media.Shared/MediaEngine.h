//
// Tomato Media
// Media Engine
// 作者：SunnyCase
// 创建时间：2015-08-06
//
#pragma once
#include "SourceReader.h"
#include "IVideoRender.h"

DEFINE_NS_MEDIA

class MediaEngine : public std::enable_shared_from_this<MediaEngine>
{
public:
	static std::shared_ptr<MediaEngine> MakeMediaEngine();

	DEFINE_PROPERTY_GET(FrameSize, USIZE);
	USIZE get_FrameSize() const { return sourceReader->FrameSize; }
	DEFINE_PROPERTY_GET(VideoRender, WRL::ComPtr<IVideoRender>);
	WRL::ComPtr<IVideoRender> get_VideoRender() const;

	void SetMediaSource(IMFMediaSource* mediaSource);
	void Play();
private:
	MediaEngine();

	void ConfigureSourceReader(IMFMediaSource* mediaSource);
	void ConfigureMediaSink();

	HRESULT OnVideoSinkEvent(IMFAsyncResult *pAsyncResult);
	void OnVideoStreamSinkRequestSample();
	void OnVideoStreamSinkPrerolled();
private:
	WRL::ComPtr<VideoSourceReader> sourceReader;
	WRL::ComPtr<IMFMediaSink> mediaSink;
	WRL::ComPtr<IMFStreamSink> videoSink;
	WRL::ComPtr<IMFPresentationClock> presentationClock;

	WRL::ComPtr<IMFAsyncCallback> videoSinkEventCallback;
};

END_NS_MEDIA
