//
// Tomato Media
// 后台音频播放器客户端
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#include "pch.h"
#include "BackgroundMediaPlayerClient.h"
#include "ApplicationDataHelper.h"

using namespace Platform;
using namespace NS_MEDIA;
using namespace NS_MEDIA::details;
using namespace Windows::Foundation;
using namespace Windows::Media;

BackgroundMediaPlayerClient::BackgroundMediaPlayerClient(Platform::String ^ mediaPlayerHandlerTypeName)
{
	// 设置处理器类型并初始化后台音频任务
	SetSetting<settings::BackgroundMediaPlayerHandlerFullNameSetting>(mediaPlayerHandlerTypeName);
	AttachMessageListener();
}

void BackgroundMediaPlayerClient::AttachMessageListener()
{
	messageListenerToken = Playback::BackgroundMediaPlayer::MessageReceivedFromBackground += ref new EventHandler<Playback::MediaPlayerDataReceivedEventArgs ^>(
		this, &BackgroundMediaPlayerClient::OnMessageReceivedFromBackground);
}

void BackgroundMediaPlayerClient::DetachMessageListener()
{
	Playback::BackgroundMediaPlayer::MessageReceivedFromBackground -= messageListenerToken;
}

void BackgroundMediaPlayerClient::OnMessageReceivedFromBackground(Object ^ sender, Playback::MediaPlayerDataReceivedEventArgs ^ e)
{
	//MessageReceivedFromBackground(sender, e->Data);
}