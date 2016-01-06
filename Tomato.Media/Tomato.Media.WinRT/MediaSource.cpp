//
// Tomato Media
// 媒体源
// 
// 作者：SunnyCase 
// 创建日期 2015-08-05
#include "pch.h"
#include "MediaSource.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace NS_MEDIA;
using namespace concurrency;

MediaSource::MediaSource()
	:mediaSource(std::make_shared<MFMediaSourceFactory>())
{

}

MediaSource::~MediaSource()
{

}

task<void> MediaSource::OpenAsync(IRandomAccessStream ^ stream, Platform::String^ uriHint)
{
	return mediaSource->OpenAsync(stream, uriHint);
}

IAsyncOperation<MediaSource^>^ MediaSource::CreateFromStream(IRandomAccessStream ^ stream)
{
	return create_async([=]
	{
		auto mediaSource = ref new MediaSource();
		return mediaSource->OpenAsync(stream).then([=]
		{
			return mediaSource;
		});
	});
}

IAsyncOperation<MediaSource^>^ MediaSource::CreateFromStream(IRandomAccessStream ^ stream, Platform::String^ uriHint)
{
	return create_async([=]
	{
		auto mediaSource = ref new MediaSource();
		return mediaSource->OpenAsync(stream, uriHint).then([=]
		{
			return mediaSource;
		});
	});
}

String^ MediaSource::Title::get()
{
	auto title(mediaSource->Title);
	return ref new String(title.c_str(), title.length());
}

String^ MediaSource::AlbumArtist::get()
{
	auto albumArtist(mediaSource->AlbumArtist);
	return ref new String(albumArtist.c_str(), albumArtist.length());
}

String^ MediaSource::Artist::get()
{
	auto artist(mediaSource->Artist);
	return ref new String(artist.c_str(), artist.length());
}

String^ MediaSource::Album::get()
{
	auto album(mediaSource->Album);
	return ref new String(album.c_str(), album.length());
}

Platform::IBox<TimeSpan>^ MediaSource::Duration::get()
{
	auto duration = mediaSource->Duration;
	if (duration == -1)
		return nullptr;
	return TimeSpan{ duration };
}

String^ MediaSource::Lyrics::get()
{
	auto lyrics(mediaSource->Lyrics);
	return ref new String(lyrics.c_str(), lyrics.length());
}