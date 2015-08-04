//
// Tomato Media
// 后台媒体播放器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#include "pch.h"
#include "BackgroundMediaPlayer.h"
#include "ApplicationDataHelper.h"
#include "IBackgroundMediaPlayerHandler.h"

using namespace Platform;
using namespace NS_MEDIA;
using namespace NS_MEDIA::details;
using namespace Windows::Media;
using namespace Windows::Foundation;

BackgroundMediaPlayer::BackgroundMediaPlayer()
{

}

void BackgroundMediaPlayer::Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance)
{
	ActivateHandler();
	AttachEventHandlers();

	deferral = taskInstance->GetDeferral();
}

void BackgroundMediaPlayer::ActivateHandler()
{
	auto audioHandlerName = GetResetSetting<settings::BackgroundMediaPlayerHandlerFullNameSetting>();

	Object^ handlerObj;
	ThrowIfFailed(RoActivateInstance(reinterpret_cast<HSTRING>(audioHandlerName),
		reinterpret_cast<IInspectable**>(&handlerObj)));
	auto audioHandler = safe_cast<IBackgroundMediaPlayerHandler^>(handlerObj);
	audioHandler->OnActivated(this);
}

void BackgroundMediaPlayer::AttachEventHandlers()
{
	Playback::BackgroundMediaPlayer::MessageReceivedFromForeground += ref new EventHandler<Playback::MediaPlayerDataReceivedEventArgs ^>(
		this, &BackgroundMediaPlayer::OnMessageReceivedFromForeground);

	mediaPlayer = Playback::BackgroundMediaPlayer::Current;
	mediaPlayer->AudioCategory = Playback::MediaPlayerAudioCategory::Media;
}


void BackgroundMediaPlayer::OnMessageReceivedFromForeground(Platform::Object ^sender, Playback::MediaPlayerDataReceivedEventArgs ^args)
{

}
