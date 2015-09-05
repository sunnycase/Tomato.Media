//
// Tomato Media Core
// 调试器日志记录追加器接口
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#pragma once
#include "IAppender.h"

DEFINE_NS_CORE_INTERN

// 调试器日志记录追加器接口
class DebuggerAppender : public IAppender
{
	virtual void Information(const std::wstring& message) override;
};

END_NS_CORE_INTERN