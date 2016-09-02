//
// Tomato Media
// 媒体元数据
// 
// 作者：SunnyCase 
// 创建时间：2015-03-18
#include "pch.h"
#include <Tomato.Media/MediaMetadata.h>
#include "../Utilities/MFMediaSourceFactory.h"
#include "id3v1.h"
#include "id3v2.h"

using namespace NS_MEDIA;
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

task<void> NS_MEDIA::TryGetMediaMetadata(MediaMetadataContainer& container, IMFByteStream* byteStream, const std::wstring& uriHint, bool brief)
{
	try
	{
		do
		{
			WRL::ComPtr<IMFByteStream> byteStreamHolder(byteStream);
			if (await ID3V2Meta::ReadBriefMetadata(byteStream, container) && container.GetSize() >= 3)
			{
				if (!brief)
					await ID3V2Meta::ReadExtraMetadata(byteStream, container);
				break;
			}
			if (await ID3V1Meta::ReadMetadata(byteStream, container))
				break;

			MFMediaSourceFactory source;
			await source.OpenAsync(byteStream, uriHint);

			container.Add<DefaultMediaMetadatas::Title>(source.Title);
			container.Add<DefaultMediaMetadatas::Album>(source.Album);
			container.Add<DefaultMediaMetadatas::Artist>(source.Artist);
			container.Add<DefaultMediaMetadatas::AlbumArtist>(source.AlbumArtist);
		} while (false);
	}
	catch(...){}
}