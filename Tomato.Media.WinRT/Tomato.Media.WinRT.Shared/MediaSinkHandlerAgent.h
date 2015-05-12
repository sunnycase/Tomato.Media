//
// Tomato Media
// Media Sink 处理器代理
// 
// (c) SunnyCase 
// 创建日期 2015-05-13
#pragma once

namespace Tomato
{
	namespace Media
	{
		ref class AudioPlayer;

		class MediaSinkHandlerAgent : public IMediaSinkHandler
		{
		public:
			MediaSinkHandlerAgent(AudioPlayer^ audioPlayer);

			// 通过 IMediaSinkHandler 继承
			virtual void OnStatusChanged(MediaSinkStatus status) override;
			virtual void OnMediaOpened() override;
			virtual void OnMediaEnded() override;
			virtual void OnSeekCompleted() override;
			virtual void OnError(HRESULT error) override;
		private:
			AudioPlayer^ GetPlayer() const;
		private:
			Platform::WeakReference playerWeak;
		};
	}
}