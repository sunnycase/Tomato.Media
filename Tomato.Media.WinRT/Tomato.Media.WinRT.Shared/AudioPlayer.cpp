#include "pch.h"
#include "AudioPlayer.h"

using namespace Tomato::Media;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Media;
using namespace Windows::UI::Core;
using namespace concurrency;

AudioPlayer::AudioPlayer()
	:uiDispatcher(Windows::UI::Xaml::Window::Current->Dispatcher),
	mediaControls(SystemMediaTransportControls::GetForCurrentView())
{
	sink = CreateWASAPIMediaSink();
	WeakReference wr(this);
	sink->SetStateChangedCallback([wr](MediaSinkState state)
	{
		auto player = wr.Resolve<AudioPlayer>();
		player->OnMediaSinkStateChanged(state);
	});
}

IAsyncAction ^ AudioPlayer::Initialize()
{
	InitializeMediaTransportControls();
	return create_async([this]
	{
		sink->Initialize();
	});
}

void AudioPlayer::SetMediaSource(MediaSource^ source)
{
	OnSetMediaSource(source);
	auto reader = CreateMFSourceReader(source->Get());
	sink->SetMediaSourceReader(std::move(reader));
}

void AudioPlayer::StartPlayback()
{
	sink->StartPlayback();
}

void AudioPlayer::PausePlayback()
{
	sink->PausePlayback();
}

void AudioPlayer::StopPlayback()
{
	sink->StopPlayback();
}

void AudioPlayer::InitializeMediaTransportControls()
{
	mediaControls->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls ^,
		SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &AudioPlayer::OnButtonPressed);
	mediaControls->IsPlayEnabled = true;
	mediaControls->IsPauseEnabled = true;
	mediaControls->IsStopEnabled = true;
	mediaControls->IsNextEnabled = false;

	mediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;
	mediaControls->DisplayUpdater->Type = MediaPlaybackType::Music;
	mediaControls->DisplayUpdater->Update();
}

bool AudioPlayer::IsSystemMediaControlEnabled::get()
{
	return mediaControls->IsEnabled;
}

void AudioPlayer::IsSystemMediaControlEnabled::set(bool value)
{
	mediaControls->IsEnabled = value;
}

void AudioPlayer::OnButtonPressed(SystemMediaTransportControls ^sender,
	SystemMediaTransportControlsButtonPressedEventArgs ^args)
{
	switch (args->Button)
	{
	case SystemMediaTransportControlsButton::Play:
		OnPlayButtonPressed(this, nullptr);
		break;
	case SystemMediaTransportControlsButton::Pause:
		OnPauseButtonPressed(this, nullptr);
		break;
	case SystemMediaTransportControlsButton::Stop:
		OnStopButtonPressed(this, nullptr);
		break;
	default:
		break;
	}
}

void AudioPlayer::UpdateMediaControls(std::function<void()>&& handler)
{
	uiDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(std::move(handler)));
}

void AudioPlayer::OnSetMediaSource(MediaSource^ source)
{
	UpdateMediaControls([=]
	{
		auto updater = mediaControls->DisplayUpdater;
		updater->MusicProperties->Title = source->Title;
		updater->MusicProperties->AlbumArtist = source->AlbumArtist;
		updater->MusicProperties->Artist = source->Artist;

		mediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;
		updater->Update();
	});
}

void AudioPlayer::OnMediaSinkStateChanging()
{
	UpdateMediaControls([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Changing;
	});
}

void AudioPlayer::OnMediaSinkPlaying()
{
	UpdateMediaControls([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Playing;
	});
}

void AudioPlayer::OnMediaSinkPaused()
{
	UpdateMediaControls([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Paused;
	});
}

void AudioPlayer::OnMediaSinkStopped()
{
	UpdateMediaControls([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Stopped;
	});
}

void AudioPlayer::OnMediaSinkStateChanged(MediaSinkState state)
{
	switch (state)
	{
	case MediaSinkState::StartPlaying:
		OnMediaSinkStateChanging();
		break;
	case MediaSinkState::Playing:
		OnMediaSinkPlaying();
		break;
	case MediaSinkState::Pausing:
		OnMediaSinkStateChanging();
		break;
	case MediaSinkState::Paused:
		OnMediaSinkPaused();
		break;
	case MediaSinkState::Stopping:
		OnMediaSinkStateChanging();
		break;
	case MediaSinkState::Stopped:
		OnMediaSinkStopped();
		break;
	default:
		break;
	}
}

bool AudioPlayer::IsPlayEnabled::get()
{
	return mediaControls->IsPlayEnabled;
}

void AudioPlayer::IsPlayEnabled::set(bool value)
{
	mediaControls->IsPlayEnabled = value;
	mediaControls->DisplayUpdater->Update();
}

bool AudioPlayer::IsNextEnabled::get()
{
	return mediaControls->IsNextEnabled;
}

void AudioPlayer::IsNextEnabled::set(bool value)
{
	mediaControls->IsNextEnabled = value;
	mediaControls->DisplayUpdater->Update();
}