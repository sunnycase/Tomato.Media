//
// Tomato Media
// 后台媒体播放器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#pragma once
#include "common.h"

DEFINE_NS_MEDIA

///<summary>后台媒体播放器</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class BackgroundMediaPlayer sealed : public Windows::ApplicationModel::Background::IBackgroundTask
{
public:
	BackgroundMediaPlayer();

	///<summary>获取播放器状态。</summary>
	property Windows::Media::Playback::MediaPlayerState State
	{
		Windows::Media::Playback::MediaPlayerState get() { return playerState; }
	}

	// 通过 IBackgroundTask 继承
	virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance);
private:
	void ActivateHandler();
	void AttachEventHandlers();

	void OnMessageReceivedFromForeground(Platform::Object ^sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^args);
private:
	Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> deferral;
	Windows::Media::Playback::MediaPlayerState playerState = Windows::Media::Playback::MediaPlayerState::Closed;
	Windows::Media::Playback::MediaPlayer^ mediaPlayer;
};

END_NS_MEDIA