//
// Tomato Media
// 工作线程
// 
// 作者：SunnyCase
// 创建日期 2015-03-15
#pragma once
#include "Tomato.Core.h"

DEFINE_NS_CORE

// 工作线程
struct TOMATO_CORE_API __declspec(novtable) WorkerThread
{
	// 执行一次
	virtual void Execute() = 0;
	// 等待事件后执行一次
	virtual void Execute(const WRL::Wrappers::Event& event) = 0;
	// 取消
	virtual void Cancel() = 0;
};

END_NS_CORE