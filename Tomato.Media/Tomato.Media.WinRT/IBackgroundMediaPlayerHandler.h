//
// Tomato Media
// 后台媒体播放器处理器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#pragma once
#include "common.h"

DEFINE_NS_MEDIA

ref class BackgroundMediaPlayer;

///<summary>后台媒体播放器处理器</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class IBackgroundMediaPlayerHandler
{
	///<summary>播放器激活时触发。</summary>
	void OnActivated(BackgroundMediaPlayer^ mediaPlayer);
	void OnReceiveMessage(Platform::String^ tag, Platform::String^ message);
	void OnCanceled();
};

END_NS_MEDIA