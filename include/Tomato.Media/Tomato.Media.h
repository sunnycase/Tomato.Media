//
// Tomato Media
// 公共头文件
// 作者：SunnyCase
// 创建时间：2015-08-04
//
#pragma once
#include <Tomato.Core/Tomato.Core.h>

#define DEFINE_NS_MEDIA namespace Tomato { namespace Media {
#define END_NS_MEDIA }}
#define NS_MEDIA Tomato::Media

#define DEFINE_NS_MEDIA_CODEC namespace Tomato { namespace Media { namespace Codec {
#define END_NS_MEDIA_CODEC }}}
#define NS_MEDIA_CODEC Tomato::Media::Codec

#ifndef DEFINE_NS_ONLY

#ifdef TOMATO_MEDIA_EXPORTS
#define TOMATO_MEDIA_API __declspec(dllexport)
#else
#define TOMATO_MEDIA_API __declspec(dllimport)
#endif

#endif