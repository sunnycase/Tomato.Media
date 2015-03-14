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

#define THROW_IF_NOT(expr, message) if(!(expr)){throw std::exception(message);}

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