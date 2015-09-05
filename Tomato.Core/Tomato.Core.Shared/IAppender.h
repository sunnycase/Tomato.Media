//
// Tomato Media Core
// 日志记录追加器接口
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#pragma once
#include "common.h"

DEFINE_NS_CORE_INTERN

// 日志记录追加器接口
struct IAppender
{
	virtual void Information(const std::wstring& message) = 0;
};

END_NS_CORE_INTERN