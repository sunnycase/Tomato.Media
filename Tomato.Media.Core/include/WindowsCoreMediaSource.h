//
// Tomato Media
// Windows.Media.Core.IMediaSource 实现
// 
// (c) SunnyCase 
// 创建日期 2015-05-07
#pragma once
#ifdef __cplusplus_winrt

#include "platform.h"
#include "IMediaSource.h"

NSDEF_TOMATO_MEDIA

// 创建 Windows.Media.Core.IMediaSource 的新实例
MEDIA_CORE_API Windows::Media::Core::IMediaSource^ __stdcall CreateWindowsCoreMediaSource(const std::wstring& url);

NSED_TOMATO_MEDIA

#endif