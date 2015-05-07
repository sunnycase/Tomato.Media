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
	:AudioPlayer(Windows::UI::Xaml::Window::Current->Dispatcher,
	SystemMediaTransportControls::GetForCurrentView())
{
}

AudioPlayer::AudioPlayer(CoreDispatcher^ uiDispatcher, SystemMediaTransportControls^ mediaControls)
	:uiDispatcher(uiDispatcher), mediaControls(mediaControls)
{
	sink = CreateWASAPIMediaSink();
	WeakReference wr(this);
	sink->SetStateChangedCallback([wr](MediaSinkState state)
	{
		auto player = wr.Resolve<AudioPlayer>();
		player->OnMediaSinkStateChanged(state);
	});
}

AudioPlayer::~AudioPlayer()
{

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
	if (source)
	{
		auto reader = CreateMFSourceReader(source->Get());
		sink->SetMediaSourceReader(std::move(reader));
	}
	else
		sink->SetMediaSourceReader(nullptr);
}

void AudioPlayer::StartPlayback()
{
	sink->StartPlayback();
}

void AudioPlayer::StartPlayback(TimeSpan time)
{
	sink->StartPlayback(time.Duration);
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
	mediaControls->IsStopEnabled = false;

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
	auto button = args->Button;
	RunOnUIDispatcher([=]
	{
		switch (args->Button)
		{
		case SystemMediaTransportControlsButton::Play:
			PlayButtonPressed(this, nullptr);
			break;
		case SystemMediaTransportControlsButton::Pause:
			PauseButtonPressed(this, nullptr);
			break;
		case SystemMediaTransportControlsButton::Stop:
			StopButtonPressed(this, nullptr);
			break;
		case SystemMediaTransportControlsButton::Previous:
			PreviousButtonPressed(this, nullptr);
			break;
		case SystemMediaTransportControlsButton::Next:
			NextButtonPressed(this, nullptr);
			break;
		default:
			break;
		}
	});
}

void AudioPlayer::RunOnUIDispatcher(std::function<void()>&& handler)
{
	if (uiDispatcher)
		uiDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(std::move(handler)));
	else
		handler();
}

void AudioPlayer::OnSetMediaSource(MediaSource^ source)
{
	RunOnUIDispatcher([=]
	{
		auto updater = mediaControls->DisplayUpdater;
		if (source)
		{
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
	});
}

void AudioPlayer::OnMediaSinkStateChanging()
{
	RunOnUIDispatcher([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Changing;
	});
}

void AudioPlayer::OnMediaSinkPlaying()
{
	RunOnUIDispatcher([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Playing;
		mediaControls->DisplayUpdater->Update();
	});
}

void AudioPlayer::OnMediaSinkPaused()
{
	RunOnUIDispatcher([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Paused;
	});
}

void AudioPlayer::OnMediaSinkStopped()
{
	RunOnUIDispatcher([=]
	{
		mediaControls->PlaybackStatus = MediaPlaybackStatus::Stopped;
	});
}

void AudioPlayer::OnMediaPlaybackStatusChanged(MediaPlaybackStatus status)
{
	RunOnUIDispatcher([=]
	{
		MediaPlaybackStatusChanged(this, status);
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
		OnMediaPlaybackStatusChanged(MediaPlaybackStatus::Playing);
		OnMediaSinkPlaying();
		break;
	case MediaSinkState::Pausing:
		OnMediaPlaybackStatusChanged(MediaPlaybackStatus::Changing);
		OnMediaSinkStateChanging();
		break;
	case MediaSinkState::Paused:
		OnMediaPlaybackStatusChanged(MediaPlaybackStatus::Paused);
		OnMediaSinkPaused();
		break;
	case MediaSinkState::Stopping:
		OnMediaPlaybackStatusChanged(MediaPlaybackStatus::Changing);
		OnMediaSinkStateChanging();
		break;
	case MediaSinkState::Stopped:
		OnMediaPlaybackStatusChanged(MediaPlaybackStatus::Stopped);
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

bool AudioPlayer::IsPauseEnabled::get()
{
	return mediaControls->IsPauseEnabled;
}

void AudioPlayer::IsPauseEnabled::set(bool value)
{
	mediaControls->IsPauseEnabled = value;
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

bool AudioPlayer::IsPreviousEnabled::get()
{
	return mediaControls->IsPreviousEnabled;
}

void AudioPlayer::IsPreviousEnabled::set(bool value)
{
	mediaControls->IsPreviousEnabled = value;
	mediaControls->DisplayUpdater->Update();
}

TimeSpan AudioPlayer::CurrentTime::get()
{
	return TimeSpan{ sink->GetCurrentTime() };
}