//
// Tomato Media
// 媒体元数据提供程序
// 
// 作者：SunnyCase 
// 创建日期 2016-01-02
#pragma once
#include "common.h"
#include <Tomato.Media/MediaMetadata.h>

DEFINE_NS_MEDIA_TOOLKIT

public ref class Picture sealed
{
internal:
	Picture(const NS_MEDIA::Picture& picture);
public:
	property Platform::String^ MimeType {Platform::String^ get() { return _mimeType; } }
	property Platform::String^ Description {Platform::String^ get() { return _description; } }
	property Platform::String^ PictureType {Platform::String^ get() { return _pictureType; } }
	property Platform::Array<byte>^ Data {Platform::Array<byte>^ get() { return _data; } }
private:
	Platform::String^ _mimeType;
	Platform::String^ _description;
	Platform::String^ _pictureType;
	Platform::Array<byte>^ _data;
};

public ref class MediaMetadataProvider sealed
{
public:
	static Windows::Foundation::IAsyncOperation<MediaMetadataProvider^>^ CreateFromStream(Windows::Storage::Streams::IRandomAccessStream^ stream, Platform::String^ uriHint, bool brief);

	///<summary>标题</summary>
	property Platform::String^ Title {Platform::String^ get(); }
	///<summary>专辑</summary>
	property Platform::String^ Album {Platform::String^ get(); }
	///<summary>专辑艺术家</summary>
	property Platform::String^ AlbumArtist {Platform::String^ get(); }
	///<summary>艺术家</summary>
	property Platform::String^ Artist {Platform::String^ get(); }
	///<summary>长度</summary>
	property Platform::IBox<Windows::Foundation::TimeSpan>^ Duration {Platform::IBox<Windows::Foundation::TimeSpan>^ get(); }
	///<summary>歌词</summary>
	property Platform::String^ Lyrics {Platform::String^ get(); }
	///<summary>图片</summary>
	property Windows::Foundation::Collections::IVector<Picture^>^ Pictures {Windows::Foundation::Collections::IVector<Picture^>^ get(); }

	virtual ~MediaMetadataProvider();
private:
	MediaMetadataProvider();
	concurrency::task<void> LoadAsync(Windows::Storage::Streams::IRandomAccessStream^ stream, Platform::String^ uriHint, bool brief);
private:
	NS_MEDIA::MediaMetadataContainer _container;
};

END_NS_MEDIA_TOOLKIT