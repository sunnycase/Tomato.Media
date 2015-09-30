//
// Tomato Media Core
// 日志记录器
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#pragma once
#include "common.h"
#include "Logger.h"
#include "Tomato.Core_i.h"

DEFINE_NS_CORE

// 日志记录器
class Logger : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::WinRt>, ABI::NS_CORE::ILogger>
{
	InspectableClass(RuntimeClass_Tomato_Core_Logger, BaseTrust);
public:
	Logger(HSTRING typeName);

	// 通过 RuntimeClass 继承
	IFACEMETHODIMP Information(HSTRING message);
private:
	Internal::Logger logger;
};

class LoggerFactory : public WRL::ActivationFactory<ABI::NS_CORE::ILoggerFactory>
{
	InspectableClassStatic(RuntimeClass_Tomato_Core_Logger, BaseTrust);
public:
	// 通过 ActivationFactory 继承
	IFACEMETHODIMP Create(HSTRING typeName, ABI::Tomato::Core::ILogger ** logger);
};

END_NS_CORE