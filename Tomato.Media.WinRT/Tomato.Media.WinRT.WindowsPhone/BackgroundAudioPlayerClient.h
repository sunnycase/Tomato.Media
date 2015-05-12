//
// Tomato Media
// 后台音频播放器客户端
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#pragma once

namespace Tomato
{
	namespace Media
	{
		///<summary>后台音频播放器</summary>
		public ref class BackgroundAudioPlayerClient sealed
		{
		public:
			///<summary>创建 <see ref="BackgroundAudioPlayerClient"/> 的新实例。</summary>
			///<param name="audioHandlerType">实现了 <see ref="IBackgroundAudioHandler"/> 的类型。</param>
			BackgroundAudioPlayerClient(Platform::String^ audioHandlerTypeName);

			void SendMessageToBackground(Windows::Foundation::Collections::ValueSet^ valueSet);

			property Windows::Foundation::TimeSpan Position
			{
				Windows::Foundation::TimeSpan get();
			}

			property Windows::Foundation::TimeSpan Duration
			{
				Windows::Foundation::TimeSpan get();
			}

			static void SendMessageToForeground(Windows::Foundation::Collections::ValueSet^ valueSet);

			event Windows::Foundation::EventHandler<Windows::Foundation::Collections::ValueSet^>^ MessageReceivedFromBackground;
		private:
			void AttachMessageListener();
			void DetachMessageListener();

			void OnMessageReceivedFromBackground(Platform::Object^ sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs^ e);
		private:
			Windows::Foundation::EventRegistrationToken messageListenerToken;
		};
	}
}