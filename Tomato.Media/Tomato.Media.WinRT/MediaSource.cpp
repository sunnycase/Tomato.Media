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
{

}

task<void> MediaSource::OpenAsync(IRandomAccessStream ^ stream)
{
	return mediaSource.OpenAsync(stream);
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

String^ MediaSource::Title::get()
{
	auto title(mediaSource.Title);
	return ref new String(title.c_str(), title.length());
}

String^ MediaSource::AlbumArtist::get()
{
	return nullptr;
	//auto album(mediaSource.Album);
	//return ref new String(album.c_str(), album.length());
}

String^ MediaSource::Artist::get()
{
	return nullptr;
	//return ws2RTString(nativeSource->GetMetadatas()
	//	.GetOrDefault<DefaultMediaMetadatas::Artist>());
}

String^ MediaSource::Album::get()
{
	auto album(mediaSource.Album);
	return ref new String(album.c_str(), album.length());
}

TimeSpan MediaSource::Duration::get()
{
	return TimeSpan{};
	//return TimeSpan{ nativeSource->GetDuration() };
}

String^ MediaSource::Lyrics::get()
{
	return nullptr;
	//return ws2RTString(nativeSource->GetMetadatas()
	//	.GetOrDefault<DefaultMediaMetadatas::Lyrics>());
}