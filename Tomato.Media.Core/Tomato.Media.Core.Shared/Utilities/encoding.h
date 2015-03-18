//
// Tomato Media
// 字符编码
// 
// (c) SunnyCase 
// 创建日期 2015-03-18
#pragma once
#include "../include/platform.h"
#include <string>
#include <memory>

namespace Tomato
{
	static std::wstring s2ws(const std::string& str)
	{
		auto ws = std::make_unique<wchar_t[]>(str.size() + 1);
		size_t size = 0;

		mbstowcs_s(&size, ws.get(), str.size() + 1, str.data(), str.size());
		return ws.get();
	}
#ifdef _WIN32
	static std::wstring s2ws(const std::string& str, UINT codePage)
	{
		auto len = MultiByteToWideChar(codePage, 0, str.data(), str.size(), nullptr, 0);
		std::wstring ws(len, 0);
		MultiByteToWideChar(codePage, 0, str.data(), str.size(), &ws[0], len);

		return ws;
	}
#endif
}