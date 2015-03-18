//
// Tomato Media
// [Internal] MMCSS 线程
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

// MMCSS 线程
class MMCSSThread
{
public:
	MMCSSThread() {}
	virtual ~MMCSSThread() {}

	// 执行一次
	virtual void Execute() = 0;
	// 等待事件后执行一次
	virtual void Execute(const wrl::Wrappers::Event& event) = 0;
	// 取消
	virtual void Cancel() = 0;
};

NSED_TOMATO_MEDIA