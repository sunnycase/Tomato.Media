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
	std::wstring s2ws(const std::string& str);
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