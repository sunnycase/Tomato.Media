//
// Tomato Media
// 媒体元数据提供程序
// 
// 作者：SunnyCase 
// 创建日期 2016-01-02
#pragma once
#include "common.h"
#include "Metadata/MediaMetadata.h"

DEFINE_NS_MEDIA

public ref class MediaMetadataProvider sealed
{
public:
	static Windows::Foundation::IAsyncOperation<MediaMetadataProvider^>^ CreateFromStream(Windows::Storage::Streams::IRandomAccessStream^ stream, bool brief);

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

	virtual ~MediaMetadataProvider();
private:
	MediaMetadataProvider();
	concurrency::task<void> LoadAsync(Windows::Storage::Streams::IRandomAccessStream^ stream, bool brief);
private:
	std::shared_ptr<Internal::MediaMetadataContainer> _container;
};

END_NS_MEDIA