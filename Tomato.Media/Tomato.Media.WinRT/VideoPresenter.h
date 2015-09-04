//
// Tomato Media
// 视频呈现器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-05
#pragma once
#include "common.h"
#include "MediaSource.h"
#include "MediaEngine.h"
#include <windows.ui.xaml.media.dxinterop.h>

DEFINE_NS_MEDIA

///<summary>视频呈现器</summary>
[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class VideoPresenter sealed
{
public:
	VideoPresenter();

	void SetMediaSource(MediaSource^ mediaSource);

	property Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ ImageSource
	{
		Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ get() { return imageSource; }
	}

	void Play();
private:
	void InitializeSurfaceImageSource();
private:
	Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ imageSource;
	WRL::ComPtr<ISurfaceImageSourceNative> sisNative;
	std::shared_ptr<MediaEngine> mediaEngine;
};

END_NS_MEDIA