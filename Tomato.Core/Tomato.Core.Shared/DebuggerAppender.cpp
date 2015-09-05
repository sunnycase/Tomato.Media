//
// Tomato Media Core
// 调试器日志记录追加器接口
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#include "pch.h"
#include "DebuggerAppender.h"

using namespace NS_CORE_INTERN;

void DebuggerAppender::Information(const std::wstring & message)
{
	OutputDebugString(message.c_str());
}
