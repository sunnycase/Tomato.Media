#pragma once
#define INITGUID
#define NOMINMAX
#include <functional>
#include <cassert>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <array>
#include <set>

#include <ppltasks.h>

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <wrl.h>
#include <windows.media.h>
#include <windows.media.core.h>

// WASAPI
#include <Audioclient.h>
#include <mmdeviceapi.h>

// Media Foundation
#include <mftransform.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>


#ifdef __cplusplus
extern "C" {
#endif
// FFmpeg
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

namespace wrl = Microsoft::WRL;