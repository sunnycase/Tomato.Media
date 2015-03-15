//
// Tomato Media
// [Internal] 媒体源
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../include/IMediaSource.h"

NSDEF_TOMATO_MEDIA

// 媒体源
class IMediaSourceIntern : public IMediaSource
{
public:
	// 创建 Media Foundation 字节流
	virtual wrl::ComPtr<IMFByteStream> CreateMFByteStream() = 0;
};

NSED_TOMATO_MEDIA