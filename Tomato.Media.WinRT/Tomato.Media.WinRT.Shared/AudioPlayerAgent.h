//
// Tomato Media
// 音频播放器代理
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#pragma once
#include "IAudioPlayer.h"

namespace Tomato
{
	namespace Media
	{
		ref class AudioPlayerAgent : public IAudioPlayer
		{
		public:
			static AudioPlayerAgent^ Create(IAudioPlayer^ player)
			{
				return ref new AudioPlayerAgent(Platform::WeakReference(player));
			}

			property Windows::Media::MediaPlaybackStatus CurrentStatus
			{
				virtual Windows::Media::MediaPlaybackStatus get()
				{
					return Player->CurrentStatus;
				}
			}

			property bool IsMediaTransportControlsEnabled
			{
				virtual bool get()
				{
					return Player->IsMediaTransportControlsEnabled;
				}

				virtual void set(bool value)
				{
					Player->IsMediaTransportControlsEnabled = value;
				}
			}

			property bool IsPlayEnabled
			{
				virtual bool get()
				{
					return Player->IsPlayEnabled;
				}

				virtual void set(bool value)
				{
					Player->IsPlayEnabled = value;
				}
			}

			property bool IsPauseEnabled
			{
				virtual bool get()
				{
					return Player->IsPauseEnabled;
				}

				virtual void set(bool value)
				{
					Player->IsPauseEnabled = value;
				}
			}


			property bool IsStopEnabled
			{
				virtual bool get()
				{
					return Player->IsStopEnabled;
				}

				virtual void set(bool value)
				{
					Player->IsStopEnabled = value;
				}
			}

			property bool IsPreviousEnabled
			{
				virtual bool get()
				{
					return Player->IsPreviousEnabled;
				}

				virtual void set(bool value)
				{
					Player->IsPreviousEnabled = value;
				}
			}

			property bool IsNextEnabled
			{
				virtual bool get()
				{
					return Player->IsNextEnabled;
				}

				virtual void set(bool value)
				{
					Player->IsNextEnabled = value;
				}
			}

			property Windows::Foundation::TimeSpan Position
			{
				virtual Windows::Foundation::TimeSpan get()
				{
					return Player->Position;
				}

				virtual void set(Windows::Foundation::TimeSpan value)
				{
					Player->Position = value;
				}
			}

			property Windows::Foundation::TimeSpan Duration
			{
				virtual Windows::Foundation::TimeSpan get()
				{
					return Player->Duration;
				}
			}

			virtual void SetMediaSource(MediaSource^ mediaSource)
			{
				Player->SetMediaSource(mediaSource);
			}

			virtual void StartPlayback()
			{
				Player->StartPlayback();
			}

			virtual void PausePlayback()
			{
				Player->PausePlayback();
			}

			virtual void StopPlayback()
			{
				Player->StopPlayback();
			}

			// 通过 IAudioPlayer 继承
			virtual event Windows::Foundation::EventHandler<Windows::Foundation::HResult>^ Error
			{
				Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Windows::Foundation::HResult>^ handler)
				{
					return Player->Error += handler;
				}

				void remove(Windows::Foundation::EventRegistrationToken token)
				{
					Player->Error -= token;
				}
			}

			virtual event Windows::Foundation::EventHandler<Windows::Media::MediaPlaybackStatus>^ MediaPlaybackStatusChanged
			{
				Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Windows::Media::MediaPlaybackStatus>^ handler)
				{
					return Player->MediaPlaybackStatusChanged += handler;
				}

				void remove(Windows::Foundation::EventRegistrationToken token)
				{
					Player->MediaPlaybackStatusChanged -= token;
				}
			}

			virtual event Windows::Foundation::EventHandler<Windows::Media::SystemMediaTransportControlsButton>^ MediaTransportButtonPressed
			{
				Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Windows::Media::SystemMediaTransportControlsButton>^ handler)
				{
					return Player->MediaTransportButtonPressed += handler;
				}

				void remove(Windows::Foundation::EventRegistrationToken token)
				{
					Player->MediaTransportButtonPressed -= token;
				}
			}

			virtual event Windows::Foundation::EventHandler<Platform::Object^>^ MediaOpened
			{
				Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Platform::Object^>^ handler)
				{
					return Player->MediaOpened += handler;
				}

				void remove(Windows::Foundation::EventRegistrationToken token)
				{
					Player->MediaOpened -= token;
				}
			}

			virtual event Windows::Foundation::EventHandler<Platform::Object^>^ MediaEnded
			{
				Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Platform::Object^>^ handler)
				{
					return Player->MediaEnded += handler;
				}

				void remove(Windows::Foundation::EventRegistrationToken token)
				{
					Player->MediaEnded -= token;
				}
			}

			virtual event Windows::Foundation::EventHandler<Platform::Object^>^ SeekCompleted
			{
				Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Platform::Object^>^ handler)
				{
					return Player->SeekCompleted += handler;
				}

				void remove(Windows::Foundation::EventRegistrationToken token)
				{
					Player->SeekCompleted -= token;
				}
			}
		private:
			AudioPlayerAgent(Platform::WeakReference player)
				:playerWeak(player)
			{

			}

			property IAudioPlayer^ Player
			{
				IAudioPlayer^ get()
				{
					auto player = playerWeak.Resolve<IAudioPlayer>();
					if (!player)
						throw ref new Platform::ObjectDisposedException();
					return player;
				}
			}

		private:
			Platform::WeakReference playerWeak;
		};
	}
}