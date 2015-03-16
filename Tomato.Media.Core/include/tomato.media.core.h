//
// Tomato Media
// Tomato Media Core
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once

#ifdef MEDIA_CORE_DLL
#define MEDIA_CORE_API __declspec(dllexport)
#else
#define MEDIA_CORE_API __declspec(dllimport)
#endif

#include "platform.h"

#ifdef _WIN32
#include "win32_exception.h"
#endif

#include <ppltasks.h>
#include <mmreg.h>
#include "IMediaSource.h"
#include "ISourceReader.h"
#include "IMediaSink.h"