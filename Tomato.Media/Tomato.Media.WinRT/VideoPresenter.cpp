//
// Tomato Media
// 视频呈现器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-05
#include "pch.h"
#include "VideoPresenter.h"
#include <d3d11.h>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

VideoPresenter::VideoPresenter()
	:mediaEngine(MediaEngine::MakeMediaEngine())
{
}

void VideoPresenter::SetMediaSource(MediaSource^ mediaSource)
{
	mediaEngine->SetMediaSource(mediaSource->MFMediaSource);
	InitializeSurfaceImageSource();
}

void VideoPresenter::InitializeSurfaceImageSource()
{
	const auto frameSize(mediaEngine->FrameSize);

	imageSource = ref new SurfaceImageSource(static_cast<int>(frameSize.Width), static_cast<int>(frameSize.Height), true);

	auto videoRender(mediaEngine->VideoRender);
	videoRender->SetSurfaceImageSource(imageSource, frameSize.Width, frameSize.Height);
}

void VideoPresenter::Play()
{
	mediaEngine->Play();
}