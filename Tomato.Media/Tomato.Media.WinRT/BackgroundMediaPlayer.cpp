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
#include "CoreMediaSource.h"

using namespace Platform;
using namespace NS_MEDIA;
using namespace NS_MEDIA::details;
using namespace Windows::Media;
using namespace Windows::Foundation;
using namespace WRL;

BackgroundMediaPlayer::BackgroundMediaPlayer()
{

}

void BackgroundMediaPlayer::Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance)
{
	AttachMessageHandlers();
	ConfigureMediaPlayer();

	deferral = taskInstance->GetDeferral();
	ActivateHandler();
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

void BackgroundMediaPlayer::AttachMessageHandlers()
{
	Playback::BackgroundMediaPlayer::MessageReceivedFromForeground += ref new EventHandler<Playback::MediaPlayerDataReceivedEventArgs ^>(
		this, &BackgroundMediaPlayer::OnMessageReceivedFromForeground);
}

void BackgroundMediaPlayer::ConfigureMediaPlayer()
{
	mediaPlayer = Playback::BackgroundMediaPlayer::Current;
	mediaPlayer->AudioCategory = Playback::MediaPlayerAudioCategory::Media;
	mediaPlayer->AudioDeviceType = Playback::MediaPlayerAudioDeviceType::Multimedia;
	mediaPlayer->AutoPlay = false;

	mediaPlayer->MediaOpened += ref new Windows::Foundation::TypedEventHandler<Playback::MediaPlayer ^, Object ^>(
		this, &BackgroundMediaPlayer::OnMediaOpened);
	mediaPlayer->CurrentStateChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &BackgroundMediaPlayer::OnCurrentStateChanged);
}


void BackgroundMediaPlayer::OnMessageReceivedFromForeground(Platform::Object ^sender, Playback::MediaPlayerDataReceivedEventArgs ^args)
{

}

void BackgroundMediaPlayer::SetMediaSource(MediaSource^ mediaSource)
{
	auto coreSource = Make<CoreMediaSource>(mediaSource->MFMediaSource);
	mediaPlayer->SetMediaSource(reinterpret_cast<Windows::Media::Core::IMediaSource^>(coreSource.Get()));
}

void BackgroundMediaPlayer::Play()
{
	mediaPlayer->Play();
}

void BackgroundMediaPlayer::Pause()
{
	mediaPlayer->Pause();
}

void BackgroundMediaPlayer::OnMediaOpened(Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	MediaOpened(this, args);
}

void BackgroundMediaPlayer::OnCurrentStateChanged(Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	CurrentStateChanged(this, args);

}
