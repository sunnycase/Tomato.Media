//
// Tomato Media Core
// 日志记录器
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#pragma once
#include "common.h"
#include <string>

DEFINE_NS_CORE_INTERN

// 日志记录器实现
class Logger
{
public:
	Logger(std::wstring typeName);

	void Information(const std::wstring& message);
private:
	const std::wstring typeName;
};

END_NS_CORE_INTERN