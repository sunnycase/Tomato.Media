//
// Tomato Media Core
// 日志记录器
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#pragma once
#include "common.h"
#include "Logger.h"

DEFINE_NS_CORE

///<summary>日志记录器</summary>
public ref class Logger sealed
{
public:
	Logger(Platform::String^ typeName);

	void Information(Platform::String^ message);
private:
	Internal::Logger logger;
};

END_NS_CORE