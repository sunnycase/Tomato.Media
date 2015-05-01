//
// Tomato Media
// 字符编码
// 
// (c) SunnyCase 
// 创建日期 2015-03-26
#include "pch.h"
#include "encoding.h"

using namespace NS_TOMATO;
using namespace wrl;
using namespace concurrency;

std::wstring Tomato::s2ws(const std::string& str)
{
	auto u = (wchar_t*)str.c_str();
	auto src = s2ws(str, CP_UTF8);
	auto dst = ws2s(src, CP_ACP);
	if (src.size() && std::all_of(src.begin(), src.end(), [](wchar_t c) {return c < 256; }))
	{
		auto data = std::make_unique<byte[]>(src.size() + 1);
		auto ptr = data.get();
		for (auto c : src)
			*ptr++ = (byte)c;
		auto result = s2ws((char*)data.get(), CP_ACP);
		return result;
	}
	return src;
}