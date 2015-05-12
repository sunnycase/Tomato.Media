//
// Tomato Media
// Tomato Media Core
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once

#include "platform.h"

#ifdef _WIN32
#include "win32_exception.h"
#endif

#include <ppltasks.h>
#include <mmreg.h>
#include "any.h"
#include "MediaMetadata.h"
#include "IMediaSource.h"
#include "WindowsCoreMediaSource.h"
#include "ISourceReader.h"
#include "IMediaSink.h"
#include "Lyrics.h"