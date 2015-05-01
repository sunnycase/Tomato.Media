//
// Tomato Media
// 内存相关
// 
// (c) SunnyCase 
// 创建日期 2015-05-01
#include "pch.h"
#include "memory.h"

void Tomato::MemoryCopy(void * dest, size_t maxCapacity, const void * src, size_t maxSize)
{
	if (maxCapacity < maxSize)
		throw std::exception("Destination buffer is not large enought.");
	auto ui64Count = maxSize / sizeof(uint64_t);
	auto rest = maxSize % sizeof(uint64_t);
	{
		auto& cntDest = reinterpret_cast<uint64_t*&>(dest);
		auto& cntSrc = reinterpret_cast<const uint64_t*&>(src);
		for (size_t i = 0; i < ui64Count; i++)
			*cntDest++ = *cntSrc++;
	}
	auto cntDest = reinterpret_cast<uint8_t*>(dest);
	auto cntSrc = reinterpret_cast<const uint8_t*>(src);
	for (size_t i = 0; i < rest; i++)
		*cntDest++ = *cntSrc++;
}
