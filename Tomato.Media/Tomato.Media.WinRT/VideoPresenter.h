//
// Tomato Media
// 视频呈现器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-05
#pragma once
#include "common.h"
#include "MediaSource.h"

DEFINE_NS_MEDIA

///<summary>视频呈现器</summary>
public ref class VideoPresenter sealed
{
public:
	VideoPresenter();

	void SetMediaSource(MediaSource^ mediaSource);
private:
	Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ imageSource;
};

END_NS_MEDIA