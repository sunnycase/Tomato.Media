//
// Tomato Media
// 平台相关
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#include "pch.h"
#include "../include/platform.h"

void DeleteCoTaskMem(void* handle) noexcept
{
	if (handle)
		CoTaskMemFree(handle);
}