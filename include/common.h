//
// Tomato Media
// 公共头文件
// 作者：SunnyCase
// 创建时间：2015-08-04
//
#pragma once

#define DEFINE_NS_MEDIA namespace Tomato { namespace Media {
#define END_NS_MEDIA }}
#define NS_MEDIA Tomato::Media

#ifdef __cplusplus_winrt
#include "winrt/platform.h"
#else
#include "desktop/platform.h"
#endif

#define DEFINE_PROPERTY_GET(name, type) __declspec(property(get = get_##name)) type name
#define ARGUMENT_NOTNULL_HR(pointer) if(!(pointer)) return E_POINTER

#include <functional>
#include <memory>

///<summary>终结器</summary>
class finalizer final
{
public:
	finalizer(std::function<void()> action)
		:action(std::move(action))
	{

	}

	~finalizer()
	{
		if (action)
			action();
	}
private:
	std::function<void()> action;
};

#ifdef _WIN32

template<class T>
struct cotaskmem_deleter
{
	void operator()(T* handle) const noexcept
	{
		CoTaskMemFree(handle);
	}
};

template<class T>
using unique_cotaskmem = std::unique_ptr<T, cotaskmem_deleter<T>>;

#endif

#include <chrono>
typedef std::ratio<1, 10000000> hn;
typedef std::chrono::duration<long long, hn> hnseconds;