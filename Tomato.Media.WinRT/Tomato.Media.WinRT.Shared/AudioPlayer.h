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

			///<summary>设置是否启用系统媒体传输控件</summary>
			property bool IsSystemMediaControlEnabled
			{
				bool get();
				void set(bool value);
			}
		private:
			void InitializeMediaTransportControls();

			void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
		private:
			std::unique_ptr<IMediaSink> sink;
			Windows::Storage::StorageFile^ file;
			Windows::Media::SystemMediaTransportControls^ mediaControls;
		};
	}
}