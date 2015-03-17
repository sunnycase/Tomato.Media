#include "pch.h"
#include "AudioPlayer.h"

using namespace Tomato::Media;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Media;
using namespace concurrency;

AudioPlayer::AudioPlayer()
{
	sink = CreateWASAPIMediaSink();
	mediaControls = SystemMediaTransportControls::GetForCurrentView();
}

IAsyncAction ^ AudioPlayer::Initialize()
{
	InitializeMediaTransportControls();
	return create_async([this]
	{
		sink->Initialize();
	});
}

IAsyncAction ^ AudioPlayer::SetMediaSource(StorageFile ^ file)
{
	this->file = file;
	return create_async([=]
	{
		return create_task(file->OpenAsync(FileAccessMode::Read))
			.then([=](Streams::IRandomAccessStream^ stream)
		{
			auto source = CreateRTMediaSource(stream);
			auto reader = CreateMFSourceReader(source.get());
			sink->SetMediaSourceReader(std::move(reader));
		});
	});
}

void AudioPlayer::StartPlayback()
{
	sink->StartPlayback();
}

void AudioPlayer::InitializeMediaTransportControls()
{
	mediaControls->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls ^,
		SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &AudioPlayer::OnButtonPressed);
	mediaControls->IsPlayEnabled = true;
	mediaControls->IsNextEnabled = true;

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
	default:
		break;
	}
}
