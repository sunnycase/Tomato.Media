//
// Tomato Media
// 公共头文件
// 作者：SunnyCase
// 创建时间：2015-08-04
//
#pragma once

#define DEFINE_NS_CORE namespace Tomato { namespace Core {
#define END_NS_CORE }}
#define NS_CORE Tomato::Core

#define DEFINE_NS_MEDIA namespace Tomato { namespace Media {
#define END_NS_MEDIA }}
#define NS_MEDIA Tomato::Media

#ifndef DEFINE_NS_ONLY

#ifdef TOMATO_CORE_EXPORTS
#define TOMATO_CORE_API __declspec(dllexport)
#else
#define TOMATO_CORE_API __declspec(dllimport)
#endif

#ifdef TOMATO_MEDIA_EXPORTS
#define TOMATO_MEDIA_API __declspec(dllexport)
#else
#define TOMATO_MEDIA_API __declspec(dllimport)
#endif

#ifdef __cplusplus_winrt
#include "winrt/platform.h"
#else
#include "desktop/platform.h"
#endif

#define DEFINE_PROPERTY_GET(name, type) __declspec(property(get = get_##name)) type name
#define ARGUMENT_NOTNULL_HR(pointer) if(!(pointer)) return E_POINTER

#include <functional>
#include <memory>
#include <type_traits>

///<summary>终结器</summary>
template<typename TCall>
class finalizer final
{
public:
	finalizer(TCall&& action)
		:action(std::forward<decltype(action)>(action))
	{

	}

	finalizer(finalizer&&) = default;
	finalizer& operator=(finalizer&&) = default;

	~finalizer()
	{
		action();
	}
private:
	TCall action;
};

template<typename TCall>
finalizer<TCall> make_finalizer(TCall&& action)
{
	return finalizer<TCall>(std::forward<decltype(action)>(action));
}

#ifdef _WIN32

template<class T>
struct cotaskmem_deleter
{
	template<typename = std::enable_if_t<std::is_array<T>::value>>
	void operator()(T handle) const noexcept
	{
		CoTaskMemFree(handle);
	}

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

#endif