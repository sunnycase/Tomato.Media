//
// Tomato Media Core
// 编码探测
// 作者：SunnyCase
// 创建时间：2015-12-31
//
#include "pch.h"
#include "..\..\include\EncodingDetector.h"

using namespace NS_CORE;

EncodingDetector::EncodingDetector()
{
	ThrowIfFailed(CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_multiLang)));
}

EncodingDetector::~EncodingDetector()
{
}
