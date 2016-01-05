//
// Tomato Media
// 后台媒体播放器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#pragma once
#include "common.h"
#include "IMediaPlayer.h"
#include "IBackgroundMediaPlayerHandler.h"

DEFINE_NS_MEDIA

///<summary>后台媒体播放器</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class BackgroundMediaPlayer sealed : public Windows::ApplicationModel::Background::IBackgroundTask, public IMediaPlayer
{
public:
	BackgroundMediaPlayer();

	///<summary>获取播放器状态。</summary>
	property Windows::Media::Playback::MediaPlayerState State
	{
		Windows::Media::Playback::MediaPlayerState get() { return mediaPlayer->CurrentState; }
	}

	// 通过 IBackgroundTask 继承
	virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance);

	// 通过 IMediaPlayer 继承
	virtual void SetMediaSource(MediaSource^ mediaSource);
	virtual void Play();
	virtual void Pause();

	property Windows::Media::SystemMediaTransportControls^ SystemMediaTransportControls
	{
		virtual Windows::Media::SystemMediaTransportControls^ get() { return mediaPlayer->SystemMediaTransportControls; }
	}

	property Windows::Foundation::TimeSpan Position
	{
		virtual Windows::Foundation::TimeSpan get() { return mediaPlayer->Position; }
		virtual void set(Windows::Foundation::TimeSpan value) { mediaPlayer->Position = value; }
	}

	property bool IsMuted
	{
		virtual bool get() { return mediaPlayer->IsMuted; }
		virtual void set(bool value) { mediaPlayer->IsMuted = value; }
	}

	property double Volume
	{
		virtual double get() { return mediaPlayer->Volume; }
		virtual void set(double value) { mediaPlayer->Volume = value; }
	}

	virtual event Windows::Foundation::TypedEventHandler<IMediaPlayer^, Platform::Object^>^ MediaOpened;
	virtual event Windows::Foundation::TypedEventHandler<IMediaPlayer^, Platform::Object^>^ MediaEnded;
	virtual event Windows::Foundation::TypedEventHandler<IMediaPlayer^, Windows::Media::Playback::MediaPlayerFailedEventArgs^>^ MediaFailed;
	virtual event Windows::Foundation::TypedEventHandler<IMediaPlayer^, Platform::Object^>^ CurrentStateChanged;
	virtual event Windows::Foundation::TypedEventHandler<IMediaPlayer^, Platform::Object^>^ SeekCompleted;

	void SendMessage(Platform::String^ tag, Platform::String^ message);
private:
	void ActivateHandler();
	void AttachMessageHandlers();
	void ConfigureMediaPlayer();
	void Shutdown();

	void OnMessageReceivedFromForeground(Platform::Object ^sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^args);
	void OnMediaOpened(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
	void OnCurrentStateChanged(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
private:
	Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> deferral;
	Windows::Media::Playback::MediaPlayer^ mediaPlayer;

	IBackgroundMediaPlayerHandler^ _audioHandler;
	void OnMediaEnded(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
	void OnMediaFailed(Windows::Media::Playback::MediaPlayer ^sender, Windows::Media::Playback::MediaPlayerFailedEventArgs ^args);
	void OnSeekCompleted(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
	void OnCompleted(Windows::ApplicationModel::Background::BackgroundTaskRegistration ^sender, Windows::ApplicationModel::Background::BackgroundTaskCompletedEventArgs ^args);
	void OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);

	Windows::Foundation::EventRegistrationToken _mediaOpenedRegToken, _mediaEndedRegToken, _currentStateChangedRegToken,
		_mediaFailedRegToken, _seekCompletedRegToken, _messageFromForegroundRegToken;
};

extern Platform::String^ BackgroundMediaPlayerActivatedMessageKey;
extern Platform::String^ BackgroundMediaPlayerUserMessageKey;

END_NS_MEDIA