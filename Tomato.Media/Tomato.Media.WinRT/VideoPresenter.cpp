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
{
}

void VideoPresenter::SetMediaSource(MediaSource^ mediaSource)
{
	videoEngine.SetMediaSource(mediaSource->MFMediaSource);
	InitializeSurfaceImageSource();
}

void VideoPresenter::InitializeSurfaceImageSource()
{
	const auto frameSize(videoEngine.FrameSize);

	imageSource = ref new SurfaceImageSource(static_cast<int>(frameSize.Width), static_cast<int>(frameSize.Height), true);
	ComPtr<IUnknown> unkImageSourceNative(reinterpret_cast<IUnknown*>(imageSource));
	ThrowIfFailed(unkImageSourceNative.As(&imageSourceNative));
	ThrowIfFailed(imageSourceNative->SetDevice(videoEngine.D2dDevice));
}

void VideoPresenter::Play()
{
	videoEngine.Play();
}