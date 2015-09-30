//
// Tomato Media Core
// 日志记录器
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#include "pch.h"
#include "LoggerRT.h"

using namespace NS_CORE;
using namespace WRL;
using namespace WRL::Wrappers;

ActivatableClassWithFactory(Logger, LoggerFactory);

Logger::Logger(HSTRING typeName)
	:logger(HStringToWString(typeName))
{
}

HRESULT Logger::Information(HSTRING message)
{
	try
	{
		logger.Information(HStringToWString(message));
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT LoggerFactory::Create(HSTRING typeName, ABI::Tomato::Core::ILogger ** logger)
{
	try
	{
		auto myLogger = Make<Logger>(typeName);
		return myLogger.CopyTo(logger);
	}
	CATCH_ALL();
}
