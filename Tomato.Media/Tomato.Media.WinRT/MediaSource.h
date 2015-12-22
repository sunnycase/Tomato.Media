//
// Tomato Media
// 媒体源
// 
// 作者：SunnyCase 
// 创建日期 2015-08-05
#pragma once
#include "common.h"
#include "Utility/MFMediaSourceWrapper.h"

DEFINE_NS_MEDIA

///<summary>媒体源</summary>
public ref class MediaSource sealed
{
public:
	static Windows::Foundation::IAsyncOperation<MediaSource^>^ CreateFromStream(Windows::Storage::Streams::IRandomAccessStream^ stream);
	static Windows::Foundation::IAsyncOperation<MediaSource^>^ CreateFromStream(Windows::Storage::Streams::IRandomAccessStream^ stream, Platform::String^ uriHint);

	///<summay>加载完整元数据</summay>
	//Windows::Foundation::IAsyncAction^ InitializeFullMetadatas();

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
internal:
	property IMFMediaSource* MFMediaSource {IMFMediaSource* get() { return mediaSource.Get(); }}
private:
	MediaSource();
	concurrency::task<void> OpenAsync(Windows::Storage::Streams::IRandomAccessStream^ stream, Platform::String^ uriHint = nullptr);
private:
	MFMediaSourceWrapper mediaSource;
};

END_NS_MEDIA