//
// Tomato Media
// 平台相关 for Windows Runtime
// 作者：SunnyCase
// 创建时间：2015-08-04
//
#pragma once
#include <comdef.h>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) throw Platform::Exception::CreateException(hr);
}

template<typename T>
inline Windows::Foundation::TimeSpan MSToTimeSpan(T ms)
{
	return Windows::Foundation::TimeSpan{ static_cast<long long>(ms * 10000) };
}