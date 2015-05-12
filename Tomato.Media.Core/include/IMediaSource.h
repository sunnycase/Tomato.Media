//
// Tomato Media
// 媒体源
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "platform.h"

struct IMFByteStream;

NSDEF_TOMATO_MEDIA

class MediaMetadataContainer;

// 媒体源
class IMediaSource
{
public:
	IMediaSource(){}
	virtual ~IMediaSource(){}

	// 加载
	virtual concurrency::task<void> Initialize() = 0;
	// 加载完整元数据
	virtual concurrency::task<void> InitializeFullMetadatas() = 0;
	// 获取元数据
	virtual const MediaMetadataContainer& GetMetadatas() const = 0;
	// 获取长度
	virtual int64_t GetDuration() = 0;
	// 创建 Media Foundation 字节流
	virtual Microsoft::WRL::ComPtr<IMFByteStream> CreateMFByteStream() = 0;
#ifdef __cplusplus_winrt
	virtual Windows::Storage::Streams::IRandomAccessStream^ CreateRTRandomAccessStream() = 0;
#endif
};

MEDIA_CORE_API std::unique_ptr<IMediaSource> __stdcall CreateRTMediaSource(Windows::Storage::Streams::IRandomAccessStream^ stream);

NSED_TOMATO_MEDIA