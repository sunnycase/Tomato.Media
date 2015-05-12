//
// Tomato Media
// Win32 异常
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once
#include "platform.h"

NSDEF_TOMATO_MEDIA

struct win32_handle_deleter
{
	void operator()(void* handle) const noexcept;
};
typedef std::unique_ptr<void, win32_handle_deleter> unique_handle;

#ifdef __cplusplus_winrt

#define THROW_WIN_IFNOT(val) { if(!(val)) throw Platform::COMException(HRESULT_FROM_WIN32(GetLastError())); }
#define THROW_IF_NOT(expr, message) if(!(expr)){throw ref new Platform::Exception(E_FAIL, message);}
#define THROW_IF_FAILED(hr) { if(FAILED(hr)) throw ref new Platform::COMException(hr); }
#define CATCH_ALL() \
catch(Platform::COMException^ ex) \
{ \
	return ex->HResult; \
} \
catch(...) \
{ \
	return E_FAIL; \
}

#define HRESULT_TO_EXCEPTION(hr) (ref new Platform::Exception(hr))

#else

#define THROW_IF_NOT(expr, message) if(!(expr)){throw std::exception(message);}
#define THROW_IF_FAILED(hr) { if(FAILED(hr)) throw ::NS_TOMATO_MEDIA::win32_exception(hr); }
#endif

NSED_TOMATO_MEDIA