//
// Tomato Media
// [Internal] Media Foundation MMCSS 提供程序
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "MMCSSThread.h"

NSDEF_TOMATO_MEDIA

// Media Foundation MMCSS 提供程序
class MFMMCSSProvider
{
public:
	MFMMCSSProvider();
	~MFMMCSSProvider() noexcept;

	std::unique_ptr<MMCSSThread> CreateMMCSSThread(std::function<void()>&& callback);
private:
	DWORD taskId, queueId;
};

NSED_TOMATO_MEDIA