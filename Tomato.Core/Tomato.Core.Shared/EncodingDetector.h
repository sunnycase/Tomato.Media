//
// Tomato Media Core
// 编码探测
// 作者：SunnyCase
// 创建时间：2015-12-31
//
#pragma once
#include "Tomato.Core/Tomato.Core.h"
#include <MLang.h>

DEFINE_NS_CORE

class EncodingDetector
{
public:
	EncodingDetector();
	~EncodingDetector();

	DWORD DetectCodePage(const std::string& text);
private:
	WRL::ComPtr<IMultiLanguage2> _multiLang;
};

END_NS_CORE