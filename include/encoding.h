//
// Tomato Media
// 字符编码
// 
// (c) SunnyCase 
// 创建日期 2015-03-18
#pragma once
#include "common.h"
#include <string>
#include <memory>

DEFINE_NS_CORE

TOMATO_CORE_API std::wstring s2ws(const std::string& str, UINT codePage);
TOMATO_CORE_API std::wstring s2ws(const std::string& str);
TOMATO_CORE_API std::string ws2s(const std::wstring& str, UINT codePage);

END_NS_CORE