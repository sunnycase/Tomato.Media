#pragma once
#include "MediaSource.h"

namespace Tomato
{
	namespace Media
	{
		///<summary>音频播放器</summary>
		public ref class AudioPlayer sealed
		{
		public:
			AudioPlayer();
			AudioPlayer(Windows::UI::Core::CoreDispatcher^ uiDispatcher, 
				Windows::Media::SystemMediaTransportControls^ mediaControls);
			virtual ~AudioPlayer();

			Windows::Foundation::IAsyncAction^ Initialize();
			Windows::Foundation::IAsyncAction^ SetMediaSource(MediaSource^ source);
			void StartPlayback();
			void StartPlayback(Windows::Foundation::TimeSpan time);
			void PausePlayback();
			void StopPlayback();

			///<summary>设置是否启用系统媒体传输控件</summary>
			property bool IsSystemMediaControlEnabled
			{
				bool get();
				void set(bool value);
			}

			property bool IsPlayEnabled
			{
				bool get();
				void set(bool value);
			}

			property bool IsPauseEnabled
			{
				bool get();
				void set(bool value);
			}

			property bool IsPreviousEnabled
			{
				bool get();
				void set(bool value);
			}

			property bool IsNextEnabled
			{
				bool get();
				void set(bool value);
			}

			property Windows::Foundation::TimeSpan CurrentTime
			{
				Windows::Foundation::TimeSpan get();
			}

			property double Volume
			{
				double get();
				void set(double value);
			}

			event Windows::Foundation::EventHandler<Platform::Object^>^ PlayButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ PauseButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ StopButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ PreviousButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ NextButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ MediaEnded;
			event Windows::Foundation::EventHandler<Windows::Media::MediaPlaybackStatus>^ MediaPlaybackStatusChanged;
		private:
			void InitializeMediaTransportControls();

			void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
			void RunOnUIDispatcher(std::function<void()>&& handler);
			void OnMediaSinkStateChanged(MediaSinkState state);

			void OnSetMediaSource(MediaSource^ source);
			void OnMediaSinkStateChanging();
			void OnMediaSinkPlaying();
			void OnMediaSinkPaused();
			void OnMediaSinkStopped();
			void OnMediaSinkEnded();
			void OnMediaPlaybackStatusChanged(Windows::Media::MediaPlaybackStatus status);
		private:
			std::unique_ptr<IMediaSink> sink;
			Windows::Media::SystemMediaTransportControls^ mediaControls;
			Windows::UI::Core::CoreDispatcher^ uiDispatcher;
		};
	}
}