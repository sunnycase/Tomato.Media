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
DEFINE_MEDIAMETA(Picture);

task<std::shared_ptr<MediaMetadataContainer>> NS_MEDIA_INTERN::GetMediaMetadata(IMFByteStream* byteStream, bool brief)
{
	auto container = std::make_shared<MediaMetadataContainer>();
	try
	{
		do
		{
			if (await ID3V2Meta::ReadBriefMetadata(byteStream, container) && container->GetSize() >= 3)
			{
				if (!brief)
					await ID3V2Meta::ReadExtraMetadata(byteStream, container);
				break;
			}
			if (await ID3V1Meta::ReadMetadata(byteStream, container))
				break;

			MFMediaSourceFactory source;
			source.Open(byteStream, L"");

			container->Add<DefaultMediaMetadatas::Title>(source.Title);
			container->Add<DefaultMediaMetadatas::Album>(source.Album);
			container->Add<DefaultMediaMetadatas::Artist>(source.Artist);
			container->Add<DefaultMediaMetadatas::AlbumArtist>(source.AlbumArtist);
		} while (false);
	}
	catch(...){}
	return container;
}