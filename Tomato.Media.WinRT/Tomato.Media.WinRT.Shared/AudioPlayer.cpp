#include "pch.h"
#include "AudioPlayer.h"

using namespace Tomato::Media;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Media;
using namespace Windows::UI::Core;
using namespace concurrency;

AudioPlayer::AudioPlayer(CoreDispatcher^ uiDispatcher, SystemMediaTransportControls^ mediaControls)
	:uiDispatcher(uiDispatcher), mediaControls(mediaControls), sinkHandler(this),
	sink(CreateWASAPIMediaSink(sinkHandler)), currentStatus(MediaPlaybackStatus::Closed)
{
	if (!uiDispatcher)
		throw ref new InvalidArgumentException(L"uiDispatcher can't be null.");
	if (!mediaControls)
		throw ref new InvalidArgumentException(L"mediaControls can't be null.");

	InitializeMediaTransportControls();
}

void AudioPlayer::SetMediaSource(MediaSource^ source)
{
	auto updater = mediaControls->DisplayUpdater;

	if (source)
	{
		sink->SetMediaSourceReader(CreateMFSourceReader(source->Get()));
		updater->MusicProperties->Title = source->Title;
		updater->MusicProperties->AlbumArtist = source->AlbumArtist;
		updater->MusicProperties->Artist = source->Artist;
	}
	else
	{
		updater->MusicProperties->Title = nullptr;
		updater->MusicProperties->AlbumArtist = nullptr;
		updater->MusicProperties->Artist = nullptr;
	}
	mediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;
	updater->Update();
}

void AudioPlayer::StartPlayback()
{
	sink->Start();
}

void AudioPlayer::PausePlayback()
{
	sink->Pause();
}

void AudioPlayer::StopPlayback()
{
	sink->Stop();
}

void AudioPlayer::InitializeMediaTransportControls()
{
	mediaControls->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls ^,
		SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &AudioPlayer::OnButtonPressed);
	mediaControls->IsStopEnabled = false;

	mediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;
	mediaControls->DisplayUpdater->Type = MediaPlaybackType::Music;
	mediaControls->DisplayUpdater->Update();
}

bool AudioPlayer::IsMediaTransportControlsEnabled::get()
{
	return mediaControls->IsEnabled;
}

void AudioPlayer::IsMediaTransportControlsEnabled::set(bool value)
{
	mediaControls->IsEnabled = value;
}

void AudioPlayer::OnButtonPressed(SystemMediaTransportControls ^sender,
	SystemMediaTransportControlsButtonPressedEventArgs ^args)
{
	MediaTransportButtonPressed(this, args->Button);
}

void AudioPlayer::RunOnUIDispatcher(std::function<void()>&& handler)
{
	uiDispatcher->RunAsync(CoreDispatcherPriority::Normal,
		ref new DispatchedHandler(std::move(handler)));
}

void AudioPlayer::OnSinkStatusChanged(MediaSinkStatus status)
{
	switch (status)
	{
	case MediaSinkStatus::Closed:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Closed);
		break;
	case MediaSinkStatus::Changing:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Changing);
		break;
	case MediaSinkStatus::Started:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Playing);
		break;
	case MediaSinkStatus::Paused:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Paused);
		break;
	case MediaSinkStatus::Stopped:
		SetMediaPlaybackStatus(MediaPlaybackStatus::Stopped);
		break;
	default:
		break;
	}
}

void AudioPlayer::OnSinkMediaOpened()
{
	MediaOpened(this, nullptr);
}

void AudioPlayer::OnSinkMediaEnded()
{
	MediaEnded(this, nullptr);
}

void AudioPlayer::OnSinkSeekCompleted()
{
	SeekCompleted(this, nullptr);
}

void AudioPlayer::OnSinkError(HRESULT error)
{
	Error(this, HResult{ error });
}

void AudioPlayer::SetMediaPlaybackStatus(MediaPlaybackStatus newStatus)
{
	if (currentStatus != newStatus)
	{
		currentStatus = newStatus;
		RunOnUIDispatcher([=]
		{
			mediaControls->PlaybackStatus = newStatus;
			MediaPlaybackStatusChanged(this, newStatus);
		});
	}
}

bool AudioPlayer::IsPlayEnabled::get()
{
	return mediaControls->IsPlayEnabled;
}

void AudioPlayer::IsPlayEnabled::set(bool value)
{
	mediaControls->IsPlayEnabled = value;
}

bool AudioPlayer::IsPauseEnabled::get()
{
	return mediaControls->IsPauseEnabled;
}

void AudioPlayer::IsPauseEnabled::set(bool value)
{
	mediaControls->IsPauseEnabled = value;
}

bool AudioPlayer::IsStopEnabled::get()
{
	return mediaControls->IsStopEnabled;
}

void AudioPlayer::IsStopEnabled::set(bool value)
{
	mediaControls->IsStopEnabled = value;
}

bool AudioPlayer::IsNextEnabled::get()
{
	return mediaControls->IsNextEnabled;
}

void AudioPlayer::IsNextEnabled::set(bool value)
{
	mediaControls->IsNextEnabled = value;
}

bool AudioPlayer::IsPreviousEnabled::get()
{
	return mediaControls->IsPreviousEnabled;
}

void AudioPlayer::IsPreviousEnabled::set(bool value)
{
	mediaControls->IsPreviousEnabled = value;
}

TimeSpan AudioPlayer::Position::get()
{
	return TimeSpan{ sink->GetPosition() };
}

void AudioPlayer::Position::set(TimeSpan position)
{
	sink->SetPosition(position.Duration);
}

TimeSpan AudioPlayer::Duration::get()
{
	return TimeSpan{ sink->GetDuration() };
}

double AudioPlayer::Volume::get()
{
	return sink->GetVolume();
}

void AudioPlayer::Volume::set(double value)
{
	sink->SetVolume(value);
}

MediaPlaybackStatus AudioPlayer::CurrentStatus::get()
{
	return currentStatus;
}