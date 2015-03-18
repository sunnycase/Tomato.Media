#pragma once

namespace Tomato
{
	namespace Media
	{
		///<summary>音频播放器</summary>
		public ref class AudioPlayer sealed
		{
		public:
			AudioPlayer();

			Windows::Foundation::IAsyncAction^ Initialize();
			Windows::Foundation::IAsyncAction^ SetMediaSource(Windows::Storage::StorageFile^ file);
			void StartPlayback();
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

			property bool IsNextEnabled
			{
				bool get();
				void set(bool value);
			}

			event Windows::Foundation::EventHandler<Platform::Object^>^ OnPlayButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ OnPauseButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ OnStopButtonPressed;
		private:
			void InitializeMediaTransportControls();

			void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
			void UpdateMediaControls(std::function<void()>&& handler);
			void OnMediaSinkStateChanged(MediaSinkState state);

			void OnSetMediaSource();
			void OnMediaSinkStateChanging();
			void OnMediaSinkPlaying();
			void OnMediaSinkPaused();
			void OnMediaSinkStopped();
		private:
			std::unique_ptr<IMediaSink> sink;
			Windows::Media::SystemMediaTransportControls^ mediaControls;
			MediaMetadataContainer metadatas;
			Windows::UI::Core::CoreDispatcher^ uiDispatcher;
		};
	}
}