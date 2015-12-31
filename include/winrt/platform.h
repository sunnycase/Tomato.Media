//
// Tomato Media
// 平台相关 for Windows Runtime
// 作者：SunnyCase
// 创建时间：2015-08-04
//
#pragma once
#include <comdef.h>
#include <wrl.h>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) throw Platform::Exception::CreateException(hr);
}

inline void ThrowIfFailed(HRESULT hr, const wchar_t* message)
{
	throw Platform::Exception::CreateException(hr, ref new Platform::String(message));
}

template<typename T>
void ThrowWin32IfNot(T value)
{
	if (!value) throw Platform::Exception::CreateException(HRESULT_FROM_WIN32(GetLastError()));
}

template<typename T, typename = std::enable_if_t<std::is_scalar<T>::value>>
void ThrowIfNot(T value, const wchar_t* message)
{
	if (!value)
		ThrowIfFailed(E_FAIL, message);
}

template<typename T, typename = std::enable_if_t<!std::is_scalar<T>::value>>
void ThrowIfNot(const T& value, const wchar_t* message)
{
	if (!value)
		ThrowIfFailed(E_FAIL, message);
}

inline void __declspec(noreturn) ThrowAlways(const wchar_t* message)
{
	ThrowIfFailed(E_FAIL, message);
}

template<typename T>
inline Windows::Foundation::TimeSpan MSToTimeSpan(T ms)
{
	return Windows::Foundation::TimeSpan{ static_cast<long long>(ms * 10000) };
}

#define CATCH_ALL() catch(Platform::Exception^ ex){ return ex->HResult;}catch(_com_error& ex){return ex.Error();}catch(...){return E_FAIL;}
#define CATCH_ALL_WITHHR(hr) catch(Platform::Exception^ ex){ hr = ex->HResult; }catch(_com_error& ex){ hr = ex.Error(); }catch(...){ hr = E_FAIL; }
#define CATCH_ALL_WITHEVENT(event) catch(Platform::Exception^ ex){ event.set_exception(ex); }catch(_com_error& ex){event.set_exception(ex);}catch(...){event.set_exception(E_FAIL);}

namespace WRL = Microsoft::WRL;