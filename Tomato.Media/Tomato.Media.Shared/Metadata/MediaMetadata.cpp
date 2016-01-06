//
// Tomato Media
// 媒体元数据
// 
// 作者：SunnyCase 
// 创建时间：2015-03-18
#include "pch.h"
#include "MediaMetadata.h"
#include "Utility/MFMediaSourceFactory.h"
#include "id3v1.h"
#include "id3v2.h"

using namespace NS_MEDIA_INTERN;
using namespace concurrency;

#define DEFINE_MEDIAMETA(name) const std::wstring DefaultMediaMetadatas:: ## name ## ::Name = L#name

DEFINE_MEDIAMETA(Title);
DEFINE_MEDIAMETA(Album);
DEFINE_MEDIAMETA(Artist);
DEFINE_MEDIAMETA(AlbumArtist);
DEFINE_MEDIAMETA(Year);
DEFINE_MEDIAMETA(TrackNumber);
DEFINE_MEDIAMETA(Genre);
DEFINE_MEDIAMETA(Lyrics);

task<std::shared_ptr<MediaMetadataContainer>> NS_MEDIA_INTERN::GetMediaMetadata(IMFByteStream* byteStream, bool brief)
{
	auto container = std::make_shared<MediaMetadataContainer>();
	WRL::ComPtr<IMFByteStream> stream(byteStream);
	// ID3V2
	return ID3V2Meta::ReadBriefMetadata(stream.Get(), container)
		.then([=](bool good)
	{
		if (good && container->GetSize() >= 3)
		{
			if (!brief)
				return ID3V2Meta::ReadExtraMetadata(stream.Get(), container)
				.then([](bool) {});
			return task_from_result();
		}
		// ID3V1
		return ID3V1Meta::ReadMetadata(stream.Get(), container)
			.then([=](bool good)
		{
			if (!good)
			{
				MFMediaSourceFactory source;
				source.Open(stream.Get(), L"");

				container->Add<DefaultMediaMetadatas::Title>(source.Title);
				container->Add<DefaultMediaMetadatas::Album>(source.Album);
				container->Add<DefaultMediaMetadatas::Artist>(source.Artist);
				container->Add<DefaultMediaMetadatas::AlbumArtist>(source.AlbumArtist);
				//container->Add<DefaultMediaMetadatas::Year>(_wtoi(source));
				//container->Add<DefaultMediaMetadatas::TrackNumber>(frame->GetText());
				//container->Add<DefaultMediaMetadatas::Genre>(frame->GetText());
			}
		});
	}).then([=]
	{
		return container;
	}).then([](task<std::shared_ptr<MediaMetadataContainer>> t)
		-> std::shared_ptr<MediaMetadataContainer>
	{
		try
		{
			return t.get();
		}
		catch (...) {}
		return nullptr;
	});
}