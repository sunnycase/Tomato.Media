//
// Tomato Media
// 后台音频播放器客户端
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#include "pch.h"
#include "BackgroundAudioPlayerClient.h"
#include "BackgroundAudioIPC.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Media::Playback;
using namespace Tomato::Media;
using namespace Tomato::Media::details;

BackgroundAudioPlayerClient::BackgroundAudioPlayerClient(String^ audioHandlerTypeName)
{
	// 设置处理器类型并初始化后台音频任务
	SetSetting<settings::HandlerFullNameSetting>(audioHandlerTypeName);
	AttachMessageListener();
}

void BackgroundAudioPlayerClient::SendMessageToBackground(ValueSet^ valueSet)
{
	BackgroundMediaPlayer::SendMessageToBackground(valueSet);
}

void BackgroundAudioPlayerClient::SendMessageToForeground(ValueSet^ valueSet)
{
	BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundAudioPlayerClient::AttachMessageListener()
{
	messageListenerToken = BackgroundMediaPlayer::MessageReceivedFromBackground +=
		ref new EventHandler<MediaPlayerDataReceivedEventArgs ^>(this,
			&BackgroundAudioPlayerClient::OnMessageReceivedFromBackground);
}

void BackgroundAudioPlayerClient::DetachMessageListener()
{
	BackgroundMediaPlayer::MessageReceivedFromBackground -= messageListenerToken;
}

void BackgroundAudioPlayerClient::OnMessageReceivedFromBackground(Object ^ sender, MediaPlayerDataReceivedEventArgs ^ e)
{
	MessageReceivedFromBackground(sender, e->Data);
}

TimeSpan BackgroundAudioPlayerClient::Position::get()
{
	return BackgroundMediaPlayer::Current->Position;
}

TimeSpan BackgroundAudioPlayerClient::Duration::get()
{
	return BackgroundMediaPlayer::Current->NaturalDuration;
}