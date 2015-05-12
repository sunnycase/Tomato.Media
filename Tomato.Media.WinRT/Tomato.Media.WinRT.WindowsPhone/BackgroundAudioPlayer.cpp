//
// Tomato Media
// 后台音频播放器
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#include "pch.h"
#include "BackgroundAudioPlayer.h"
#include "BackgroundAudioIPC.h"
#include "AudioPlayerAgent.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Media;
using namespace Windows::Media::Playback;
using namespace Tomato::Media;
using namespace Tomato::Media::details;
using namespace concurrency;

BackgroundAudioPlayer::BackgroundAudioPlayer()
	:currentStatus(MediaPlaybackStatus::Closed)
{
	RegisterMFTs();
}

MediaPlaybackStatus BackgroundAudioPlayer::CurrentStatus::get()
{
	return currentStatus;
}

bool BackgroundAudioPlayer::IsMediaTransportControlsEnabled::get()
{
	return mediaTransportControls->IsEnabled;
}

void BackgroundAudioPlayer::IsMediaTransportControlsEnabled::set(bool value)
{
	mediaTransportControls->IsEnabled = value;
}

bool BackgroundAudioPlayer::IsPlayEnabled::get()
{
	return mediaTransportControls->IsPlayEnabled;
}

void BackgroundAudioPlayer::IsPlayEnabled::set(bool value)
{
	mediaTransportControls->IsPlayEnabled = value;
}

bool BackgroundAudioPlayer::IsPauseEnabled::get()
{
	return mediaTransportControls->IsPauseEnabled;
}

void BackgroundAudioPlayer::IsPauseEnabled::set(bool value)
{
	mediaTransportControls->IsPauseEnabled = value;
}

bool BackgroundAudioPlayer::IsStopEnabled::get()
{
	return mediaTransportControls->IsStopEnabled;
}

void BackgroundAudioPlayer::IsStopEnabled::set(bool value)
{
	mediaTransportControls->IsStopEnabled = value;
}

bool BackgroundAudioPlayer::IsPreviousEnabled::get()
{
	return mediaTransportControls->IsPreviousEnabled;
}

void BackgroundAudioPlayer::IsPreviousEnabled::set(bool value)
{
	mediaTransportControls->IsPreviousEnabled = value;
}

bool BackgroundAudioPlayer::IsNextEnabled::get()
{
	return mediaTransportControls->IsNextEnabled;
}

void BackgroundAudioPlayer::IsNextEnabled::set(bool value)
{
	mediaTransportControls->IsNextEnabled = value;
}

TimeSpan BackgroundAudioPlayer::Position::get()
{
	return BackgroundMediaPlayer::Current->Position;
}

void BackgroundAudioPlayer::Position::set(TimeSpan value)
{
	if (currentStatus == MediaPlaybackStatus::Changing ||
		currentStatus == MediaPlaybackStatus::Closed || seeking)
		throw ref new Exception(E_NOT_VALID_STATE);

	if (value.Duration > Duration.Duration)
		throw ref new InvalidArgumentException(L"value is out of range.");

	if (BackgroundMediaPlayer::Current->Position.Duration != value.Duration)
	{
		seeking = true;
		BackgroundMediaPlayer::Current->Position = value;
	}
	else
	{
		SeekCompleted(this, nullptr);
		audioHandler->OnSeekCompleted();
	}
}

TimeSpan BackgroundAudioPlayer::Duration::get()
{
	return BackgroundMediaPlayer::Current->NaturalDuration;
}

void BackgroundAudioPlayer::SetMediaSource(MediaSource^ mediaSource)
{
	// 状态必须为 Closed 或 Stopped
	if ((currentStatus != MediaPlaybackStatus::Closed &&
		currentStatus != MediaPlaybackStatus::Stopped) || seeking)
		throw ref new Exception(E_NOT_VALID_STATE);

	SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);

	auto stream = mediaSource->Get()->CreateRTRandomAccessStream();
	BackgroundMediaPlayer::Current->SetStreamSource(stream);
	auto updater = mediaTransportControls->DisplayUpdater;

	updater->MusicProperties->Title = mediaSource->Title;
	updater->MusicProperties->AlbumArtist = mediaSource->AlbumArtist;
	updater->MusicProperties->Artist = mediaSource->Artist;

	mediaTransportControls->PlaybackStatus = MediaPlaybackStatus::Closed;
	updater->Update();
}

void BackgroundAudioPlayer::StartPlayback()
{
	// 状态必须为 Paused 或 Stopped
	if ((currentStatus != MediaPlaybackStatus::Paused &&
		currentStatus != MediaPlaybackStatus::Stopped) || seeking ||
		BackgroundMediaPlayer::Current->Position.Duration ==
		BackgroundMediaPlayer::Current->NaturalDuration.Duration)
		throw ref new Exception(E_NOT_VALID_STATE);

	SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);
	BackgroundMediaPlayer::Current->Play();
}

void BackgroundAudioPlayer::PausePlayback()
{
	// 状态必须为 Playing
	if ((currentStatus != MediaPlaybackStatus::Playing) || seeking)
		throw ref new Exception(E_NOT_VALID_STATE);

	if (BackgroundMediaPlayer::Current->CanPause)
	{
		SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);
		BackgroundMediaPlayer::Current->Pause();
	}
	else
		throw ref new Exception(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));
}

void BackgroundAudioPlayer::StopPlayback()
{
	// 状态必须为 Playing 或 Paused
	if (!seeking)
	{
		if (currentStatus == MediaPlaybackStatus::Playing)
		{
			if (BackgroundMediaPlayer::Current->CanPause)
			{
				SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);
				nextPauseIsStop = true;
				BackgroundMediaPlayer::Current->Pause();
			}
			else
				throw ref new Exception(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));
		}
		// 暂停时只调整位置
		else if (currentStatus == MediaPlaybackStatus::Paused)
			SetStopMayNeedSeek();
		else
			throw ref new Exception(E_NOT_VALID_STATE);
	}
	else
		throw ref new Exception(E_NOT_VALID_STATE);

}

void BackgroundAudioPlayer::Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^ taskInstance)
{
	ActivateAudioHandler();
	InitializeMediaTransportControls();
	AttachEventHandlers();

	// 返回接口
	audioHandler->OnActivated(AudioPlayerAgent::Create(this));
	deferral = taskInstance->GetDeferral();
}

void BackgroundAudioPlayer::ActivateAudioHandler()
{
	auto audioHandlerName = GetRestSetting<settings::HandlerFullNameSetting>();

	Object^ handlerObj;
	THROW_IF_FAILED(RoActivateInstance(reinterpret_cast<HSTRING>(audioHandlerName),
		reinterpret_cast<IInspectable**>(&handlerObj)));
	audioHandler = safe_cast<IBackgroundAudioHandler^>(handlerObj);
}

void BackgroundAudioPlayer::InitializeMediaTransportControls()
{
	mediaTransportControls = SystemMediaTransportControls::GetForCurrentView();
	mediaTransportControls->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls ^,
		SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &BackgroundAudioPlayer::OnButtonPressed);

	auto updater = mediaTransportControls->DisplayUpdater;
	updater->Type = MediaPlaybackType::Music;
}

void BackgroundAudioPlayer::AttachEventHandlers()
{
	BackgroundMediaPlayer::MessageReceivedFromForeground +=
		ref new EventHandler<MediaPlayerDataReceivedEventArgs ^>(this,
			&BackgroundAudioPlayer::OnMessageReceivedFromForeground);

	auto player = BackgroundMediaPlayer::Current;
	player->AutoPlay = false;
	player->MediaOpened += ref new TypedEventHandler<MediaPlayer ^, Object ^>(
		this, &BackgroundAudioPlayer::OnMediaOpened);
	player->MediaFailed += ref new TypedEventHandler<MediaPlayer ^, MediaPlayerFailedEventArgs ^>(
		this, &BackgroundAudioPlayer::OnMediaFailed);
	player->MediaEnded += ref new TypedEventHandler<MediaPlayer ^, Object ^>(
		this, &BackgroundAudioPlayer::OnMediaEnded);
	player->CurrentStateChanged += ref new TypedEventHandler<MediaPlayer ^, Object ^>(
		this, &BackgroundAudioPlayer::OnCurrentStateChanged);
	player->SeekCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &Tomato::Media::BackgroundAudioPlayer::OnSeekCompleted);
}

void BackgroundAudioPlayer::OnMessageReceivedFromForeground(Object ^sender, MediaPlayerDataReceivedEventArgs ^args)
{
	audioHandler->OnReceivedMessageFromForeground(sender, args->Data);
}

void BackgroundAudioPlayer::ReportOnError(concurrency::task<void> task)
{
	task.then([=](concurrency::task<void> t)
	{
		try
		{
			t.get();
		}
		catch (Exception^ ex)
		{
			Error(this, HResult{ ex->HResult });
			audioHandler->OnError(HResult{ ex->HResult });
		}
		catch (...)
		{
			Error(this, HResult{ E_UNEXPECTED });
			audioHandler->OnError(HResult{ E_UNEXPECTED });
		}
	});
}

void BackgroundAudioPlayer::OnMediaOpened(MediaPlayer ^sender, Object ^args)
{
	MediaOpened(this, nullptr);
	audioHandler->OnMediaOpened();
}

void BackgroundAudioPlayer::SetMediaPlaybackStatus(MediaPlaybackStatus newStatus)
{
	if (currentStatus != newStatus)
	{
		currentStatus = newStatus;
		mediaTransportControls->PlaybackStatus = newStatus;
		MediaPlaybackStatusChanged(this, newStatus);
		audioHandler->OnMediaPlaybackStatusChanged(newStatus);
	}
}

void BackgroundAudioPlayer::OnCurrentStateChanged(MediaPlayer ^sender, Object ^args)
{
	switch (sender->CurrentState)
	{
	case MediaPlayerState::Opening:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);
		break;
	case MediaPlayerState::Playing:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Playing);
		break;
	case MediaPlayerState::Paused:
		if (nextPauseIsStop)
		{
			nextPauseIsStop = false;
			SetStopMayNeedSeek();
		}
		else
			SetMediaPlaybackStatus(MediaPlaybackStatus::Paused);
		break;
	default:
		break;
	}
}

void BackgroundAudioPlayer::OnMediaFailed(MediaPlayer ^sender, MediaPlayerFailedEventArgs ^args)
{
	Error(this, args->ExtendedErrorCode);
	audioHandler->OnError(args->ExtendedErrorCode);
}

void BackgroundAudioPlayer::OnButtonPressed(SystemMediaTransportControls ^sender, SystemMediaTransportControlsButtonPressedEventArgs ^args)
{
	MediaTransportButtonPressed(this, args->Button);
	audioHandler->OnMediaTransportButtonPressed(args->Button);
}

void BackgroundAudioPlayer::SetStopMayNeedSeek()
{
	SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);

	// Stop 的时候需要设置 Position 为 0
	if (BackgroundMediaPlayer::Current->CanSeek &&
		BackgroundMediaPlayer::Current->Position.Duration != 0)
	{
		nextSeekIsStop = true;
		BackgroundMediaPlayer::Current->Position = TimeSpan{ 0 };
	}
	else
	{
		SetMediaPlaybackStatus(MediaPlaybackStatus::Stopped);
	}
}

void BackgroundAudioPlayer::OnSeekCompleted(MediaPlayer ^sender, Object ^args)
{
	// 一次 Stop 操作
	if (nextSeekIsStop)
	{
		nextSeekIsStop = false;
		SetMediaPlaybackStatus(MediaPlaybackStatus::Stopped);
	}
	else
	{
		seeking = false;
		SeekCompleted(this, nullptr);
		audioHandler->OnSeekCompleted();
	}
}

void BackgroundAudioPlayer::OnMediaEnded(MediaPlayer ^sender, Object ^args)
{
	BackgroundMediaPlayer::Current->AutoPlay = false;
	BackgroundMediaPlayer::Current->Pause();
	MediaEnded(this, nullptr);
	audioHandler->OnMediaEnded();
}
