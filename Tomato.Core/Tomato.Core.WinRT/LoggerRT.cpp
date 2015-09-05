//
// Tomato Media Core
// 日志记录器
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#include "pch.h"
#include "LoggerRT.h"

using namespace NS_CORE;
using namespace Platform;

Logger::Logger(String^ typeName)
	:logger(std::wstring(typeName->Begin(), typeName->End()))
{
}

void Logger::Information(String ^ message)
{
	logger.Information(std::wstring(message->Begin(), message->End()));
}
