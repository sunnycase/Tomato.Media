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

class win32_exception
{
public:
	win32_exception(int error)
		:error(error)
	{

	}

	std::wstring what() const noexcept
	{
		return format_message(error);
	}

	int code() const noexcept
	{
		return error;
	}
private:
	static std::wstring format_message(int error);

	int error;
};

#define THROW_WIN_IFNOT(val) { if(!(val)) throw ::NS_TOMATO_MEDIA::win32_exception(GetLastError()); }

#ifdef __cplusplus_winrt
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
#else

#define THROW_IF_NOT(expr, message) if(!(expr)){throw std::exception(message);}
#define THROW_IF_FAILED(hr) { if(FAILED(hr)) throw ::NS_TOMATO_MEDIA::win32_exception(hr); }
#endif

NSED_TOMATO_MEDIA