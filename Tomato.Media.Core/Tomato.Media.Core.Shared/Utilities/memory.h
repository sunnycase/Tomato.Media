//
// Tomato Media
// 内存相关
// 
// (c) SunnyCase 
// 创建日期 2015-05-01
#pragma once
#include "../include/platform.h"

namespace Tomato
{
	void MemoryCopy(void* dest, size_t maxCapacity, const void* src, size_t maxSize);
}