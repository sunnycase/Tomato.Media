//
// Tomato Media
// 不可复制基类
// 
// 作者：SunnyCase
// 创建日期 2015-08-16
#pragma once
#include "Tomato.Core.h"

DEFINE_NS_CORE

struct NonCopyable
{
	NonCopyable(){}
	NonCopyable(NonCopyable&) = delete;
	NonCopyable& operator=(NonCopyable&) = delete;
};

END_NS_CORE