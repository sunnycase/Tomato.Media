//
// Tomato Media
// 后台音频任务
// 
// (c) SunnyCase 
// 创建日期 2015-05-06
#pragma once

namespace Tomato
{
	namespace Media
	{
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP

		[Windows::Foundation::Metadata::WebHostHidden]
		public ref class BackgroundAudioTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
		{
		public:
			BackgroundAudioTask();

			// 通过 IBackgroundTask 继承
			virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance);

			///<summary>初始化</summary>
			static Windows::Foundation::IAsyncAction^ Initialize();
			///<summary>设置媒体源</summary>
			static Windows::Foundation::IAsyncAction^ SetMediaSource(Platform::String^ path);
			static void StartPlayback();

			///<summary>设置是否启用系统媒体传输控件</summary>
			property static bool IsSystemMediaControlEnabled
			{
				bool get();
				void set(bool value);
			}

			property static bool IsPlayEnabled
			{
				bool get();
				void set(bool value);
			}

			property static bool IsPauseEnabled
			{
				bool get();
				void set(bool value);
			}

			property static bool IsPreviousEnabled
			{
				bool get();
				void set(bool value);
			}

			property static bool IsNextEnabled
			{
				bool get();
				void set(bool value);
			}

			property static Windows::Foundation::TimeSpan CurrentTime
			{
				Windows::Foundation::TimeSpan get();
			}

			static event Windows::Foundation::EventHandler<Platform::Object^>^ PlayButtonPressed;
			static event Windows::Foundation::EventHandler<Platform::Object^>^ PauseButtonPressed;
			static event Windows::Foundation::EventHandler<Platform::Object^>^ StopButtonPressed;
			static event Windows::Foundation::EventHandler<Platform::Object^>^ PreviousButtonPressed;
			static event Windows::Foundation::EventHandler<Platform::Object^>^ NextButtonPressed;
			static event Windows::Foundation::EventHandler<Windows::Media::MediaPlaybackStatus>^ MediaPlaybackStatusChanged;
		private:
			void OnInitialize();
			void InitializeMediaTransportControls();
			void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
			void RunOnUIDispatcher(std::function<void()>&& handler);
			void OnSetMediaSource(Windows::Foundation::Collections::ValueSet^ value);
			void OnStartPlayback();

			void OnMediaOpened(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
			void OnCurrentStateChanged(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
			void OnMediaFailed(Windows::Media::Playback::MediaPlayer ^sender, Windows::Media::Playback::MediaPlayerFailedEventArgs ^args);

			void ReplyMessageToForeground(Platform::String^ message, uint64_t messageId);
			void ReplyMessageToForeground(Platform::String^ message, uint64_t messageId, Windows::Foundation::Collections::ValueSet^ valueSet);
			void ReplyErrorToForeground(Platform::String^ errorMessage, uint64_t messageId);
			void ReplyErrorToForeground(concurrency::task<void> t, uint64_t messageId);
			void OnMessageReceivedFromForeground(Platform::Object ^sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^args);
			void SendStatusMessageToForeground(Platform::String^ message, Windows::Foundation::Collections::ValueSet^ valueSet);
			void SendStatusMessageToForeground(Platform::String^ message);

			static void EnsureInitialized();
			static void OnMediaPlaybackStatusChanged(Windows::Media::MediaPlaybackStatus status);
		private:
			Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> Deferral;
			Windows::Media::Playback::MediaPlayer^ mediaPlayer;
			Windows::Media::SystemMediaTransportControls^ mediaTransportControls;
			concurrency::task_completion_event<void> mediaOpenedEvent;
			Windows::UI::Core::CoreDispatcher^ uiDispatcher;
		private:
			static bool initialized;
		};

#endif
	}
}