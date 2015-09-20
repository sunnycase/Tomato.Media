//
// Tomato Media Gaming
// 公共头文件
// 作者：SunnyCase
// 创建时间：2015-09-04
//
#pragma once
#include "../../include/common.h"

#define DEFINE_NS_MEDIA_GAMING namespace Tomato { namespace Media { namespace Gaming {
#define END_NS_MEDIA_GAMING }}}
#define NS_MEDIA_GAMING Tomato::Media::Gaming

#ifdef __cplusplus_winrt
namespace Win2D = Microsoft::Graphics::Canvas;
#endif