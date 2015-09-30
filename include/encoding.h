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

#ifdef _WIN32
inline std::wstring s2ws(const std::string& str, UINT codePage)
{
	auto len = MultiByteToWideChar(codePage, 0, str.data(), str.size(), nullptr, 0);
	std::wstring ws(len, 0);
	MultiByteToWideChar(codePage, 0, str.data(), str.size(), &ws[0], len);

	return ws;
}

inline std::string ws2s(const std::wstring& str, UINT codePage)
{
	auto len = WideCharToMultiByte(codePage, 0, str.data(), str.size(), nullptr, 0, nullptr, nullptr);
	std::string s(len, 0);
	WideCharToMultiByte(codePage, 0, str.data(), str.size(), &s[0], len, nullptr, nullptr);

	return s;
}
#endif

END_NS_CORE