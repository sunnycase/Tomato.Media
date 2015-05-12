//
// Tomato Media
// 音频播放器接口
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#pragma once
#include "MediaSource.h"

namespace Tomato
{
	namespace Media
	{
		///<summary>音频播放器接口</summary>
		///<remark>
		/// 音频播放器对象模型：
		/// 音频播放器基于状态机实现。
		/// 初始化后 CurrentStatus 设置为 Closed。
		/// 1. 调用 SetMediaSource 设置媒体源，设置成功 CurrentStatus 设置为 Paused，
		///    产生 MediaOpened 事件。进入步骤 2 或 5。
		/// 2. 调用 StartPlayback，CurrentStatus 设置为 Changing，
		///    开始播放后 CurrentStatus 设置为 Playing。可进入步骤 3 或 4、5、6。
		/// 3. 调用 PausePlayback，CurrentStatus 设置为 Changing，
		///    暂停后 CurrentStatus 设置为 Paused。可进入步骤 2 或 4、5。
		/// 4. 调用 StopPlayback，CurrentStatus 设置为 Changing，
		///    停止后 CurrentStatus 设置为 Stopped。可进入步骤 1 或 2、5。
		/// 5. 设置 Position 调整播放位置，设置成功产生 SeekCompleted 事件。
		/// 6. 媒体自然结束后产生 MediaEnded 事件，CurrentStatus 设置为 Paused。
		///    可进入步骤 1 或设置 非 Duration 的 Position 后进入 2。
		/// 7. 以上步骤产生错误均会产生 Error 事件。
		///</remark>
		public interface class IAudioPlayer
		{
		public:
			property Windows::Media::MediaPlaybackStatus CurrentStatus
			{
				Windows::Media::MediaPlaybackStatus get();
			}

			property bool IsMediaTransportControlsEnabled
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

			property bool IsStopEnabled
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

			property Windows::Foundation::TimeSpan Position
			{
				Windows::Foundation::TimeSpan get();
				void set(Windows::Foundation::TimeSpan value);
			}

			property Windows::Foundation::TimeSpan Duration
			{
				Windows::Foundation::TimeSpan get();
			}

			void SetMediaSource(MediaSource^ mediaSource);
			void StartPlayback();
			void PausePlayback();
			void StopPlayback();

			event Windows::Foundation::EventHandler<Windows::Foundation::HResult>^ Error;
			event Windows::Foundation::EventHandler<Windows::Media::MediaPlaybackStatus>^ MediaPlaybackStatusChanged;
			event Windows::Foundation::EventHandler<Windows::Media::SystemMediaTransportControlsButton>^ MediaTransportButtonPressed;
			event Windows::Foundation::EventHandler<Platform::Object^>^ MediaOpened;
			event Windows::Foundation::EventHandler<Platform::Object^>^ MediaEnded;
			event Windows::Foundation::EventHandler<Platform::Object^>^ SeekCompleted;
		};
	}
}