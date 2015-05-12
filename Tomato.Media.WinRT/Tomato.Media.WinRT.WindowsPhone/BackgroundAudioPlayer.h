//
// Tomato Media
// 后台音频播放器
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#pragma once
#include "IAudioPlayer.h"
#include "IBackgroundAudioHandler.h"

namespace Tomato
{
	namespace Media
	{
		// 后台音频播放器
		[Windows::Foundation::Metadata::WebHostHidden]
		public ref class BackgroundAudioPlayer sealed : public IAudioPlayer,
			public Windows::ApplicationModel::Background::IBackgroundTask
		{
		public:
			BackgroundAudioPlayer();

			property Windows::Media::MediaPlaybackStatus CurrentStatus
			{
				virtual Windows::Media::MediaPlaybackStatus get();
			}

			property bool IsMediaTransportControlsEnabled
			{
				virtual bool get();
				virtual void set(bool value);
			}

			property bool IsPlayEnabled
			{
				virtual bool get();
				virtual void set(bool value);
			}

			property bool IsPauseEnabled
			{
				virtual bool get();
				virtual void set(bool value);
			}

			property bool IsStopEnabled
			{
				virtual bool get();
				virtual void set(bool value);
			}

			property bool IsPreviousEnabled
			{
				virtual bool get();
				virtual void set(bool value);
			}

			property bool IsNextEnabled
			{
				virtual bool get();
				virtual void set(bool value);
			}

			property Windows::Foundation::TimeSpan Position
			{
				virtual Windows::Foundation::TimeSpan get();
				virtual void set(Windows::Foundation::TimeSpan value);
			}

			property Windows::Foundation::TimeSpan Duration
			{
				virtual Windows::Foundation::TimeSpan get();
			}

			virtual void SetMediaSource(MediaSource^ mediaSource);
			virtual void StartPlayback();
			virtual void PausePlayback();
			virtual void StopPlayback();

			virtual event Windows::Foundation::EventHandler<Windows::Foundation::HResult>^ Error;
			virtual event Windows::Foundation::EventHandler<Windows::Media::MediaPlaybackStatus>^ MediaPlaybackStatusChanged;
			virtual event Windows::Foundation::EventHandler<Windows::Media::SystemMediaTransportControlsButton>^ MediaTransportButtonPressed;
			virtual event Windows::Foundation::EventHandler<Platform::Object^>^ MediaOpened;
			virtual event Windows::Foundation::EventHandler<Platform::Object^>^ MediaEnded;
			virtual event Windows::Foundation::EventHandler<Platform::Object^>^ SeekCompleted;

			// 通过 IBackgroundTask 继承
			virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance);
		private:
			// 激活音频处理器
			void ActivateAudioHandler();
			void InitializeMediaTransportControls();
			void AttachEventHandlers();
			void SetMediaPlaybackStatus(Windows::Media::MediaPlaybackStatus newStatus);
			// 设置停止状态，可能需要 Seek
			void SetStopMayNeedSeek();

			void OnMessageReceivedFromForeground(Platform::Object ^sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^args);
			void OnMediaOpened(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
			void OnCurrentStateChanged(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
			void OnMediaFailed(Windows::Media::Playback::MediaPlayer ^sender, Windows::Media::Playback::MediaPlayerFailedEventArgs ^args);
			void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
			void OnSeekCompleted(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
			void OnMediaEnded(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);

			void ReportOnError(concurrency::task<void> task);
		private:
			Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> deferral;
			Windows::Media::MediaPlaybackStatus currentStatus;
			IBackgroundAudioHandler^ audioHandler;
			Windows::Media::SystemMediaTransportControls^ mediaTransportControls;
			bool nextPauseIsStop = false, nextSeekIsStop = false, seeking = false, mediaEnded = false;
		};
	}
}