//
// Tomato Media
// 后台音频处理器
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#pragma once
#include "IAudioPlayer.h"

namespace Tomato
{
	namespace Media
	{
		///<summary>后台音频处理器</summary>
		public interface class IBackgroundAudioHandler
		{
			///<summary>播放器激活后的回调</summary>
			void OnActivated(IAudioPlayer^ player);

			void OnReceivedMessageFromForeground(Platform::Object^ sender, Windows::Foundation::Collections::ValueSet^ valueSet);
			void OnError(Windows::Foundation::HResult exception);
			void OnMediaPlaybackStatusChanged(Windows::Media::MediaPlaybackStatus newStatus);
			void OnMediaTransportButtonPressed(Windows::Media::SystemMediaTransportControlsButton button);
			void OnMediaOpened();
			void OnMediaEnded();
			void OnSeekCompleted();
		};
	}
}