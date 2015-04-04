//
// Tomato Media
// 平台相关
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once
#include <inttypes.h>
#include <memory>
#include <string>
#include <ppltasks.h>

typedef unsigned char byte;

#if !defined(_WIN32)
#error "Tomato Media now support Win32 only."
#endif

#include <winapifamily.h>

#define NSDEF_TOMATO_MEDIA namespace Tomato{ namespace Media{
#define NSED_TOMATO_MEDIA }}

#define NS_TOMATO Tomato
#define NS_TOMATO_MEDIA NS_TOMATO::Media

#if _MSC_VER <= 1900
#define noexcept throw()
#endif

#ifdef _WIN32

void DeleteCoTaskMem(void* handle) noexcept;

template<class T>
struct cotaskmem_deleter
{
	void operator()(T* handle) const noexcept
	{
		DeleteCoTaskMem(handle);
	}
};

template<class T>
using unique_cotaskmem = std::unique_ptr<T, cotaskmem_deleter<T>>;

#endif

#ifdef MEDIA_CORE_DLL
#define MEDIA_CORE_API __declspec(dllexport)
#else
#define MEDIA_CORE_API __declspec(dllimport)
#endif

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) | ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) & ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a)); } \
inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^ ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
}
#endif

inline uint32_t to_be(uint32_t value) noexcept
{
	return static_cast<uint32_t>(_byteswap_ulong(static_cast<unsigned long>(value)));
}