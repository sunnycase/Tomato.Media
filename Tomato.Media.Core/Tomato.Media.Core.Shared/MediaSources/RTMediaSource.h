//
// Tomato Media
// [Internal] Windows Runtime 媒体源
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "IMediaSourceIntern.h"
#include "../include/MediaMetadata.h"

NSDEF_TOMATO_MEDIA

// Windows Runtime 媒体源
class RTMediaSource : public IMediaSourceIntern
{
public:
	RTMediaSource(Windows::Storage::Streams::IRandomAccessStream^ stream);

	virtual wrl::ComPtr<IMFByteStream> CreateMFByteStream();
#ifdef __cplusplus_winrt
	virtual Windows::Storage::Streams::IRandomAccessStream^ CreateRTRandomAccessStream();
#endif

	virtual concurrency::task<void> Initialize();
	virtual concurrency::task<void> InitializeFullMetadatas();
	virtual const MediaMetadataContainer& GetMetadatas() const;
	virtual int64_t GetDuration();
private:
	Windows::Storage::Streams::IRandomAccessStream^ stream;
	bool initialized = false;
	int64_t duration = -1;
	MediaMetadataContainer metadatas;
};

NSED_TOMATO_MEDIA