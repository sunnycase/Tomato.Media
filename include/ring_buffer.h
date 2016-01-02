//
// Tomato Core
// 环形缓冲
// 
// 作者：SunnyCase 
// 创建时间：2015-05-01
#pragma once
#include "common.h"
#include "push_undef_minmax.h"
DEFINE_NS_CORE

template<typename Elem>
class ring_buffer
{
public:
	ring_buffer() {}

	void init(size_t bufferSize)
	{
		this->bufferSize = bufferSize;
		buffer = std::make_unique<Elem[]>(bufferSize);
		bufferEnd = buffer.get() + bufferSize;
		clear();
	}

	size_t read(Elem* dest, size_t maxCount)
	{
		auto cntReadPtr = readPtr, cntWritePtr = writePtr;
		if (cntReadPtr == cntWritePtr && !bufferFull) return 0;
		else if (cntReadPtr < cntWritePtr)
		{
			maxCount = std::min(maxCount, size_t(cntWritePtr - cntReadPtr));
			auto sizeInBytes = maxCount * sizeof(Elem);
			ThrowIfNot(memcpy_s(dest, sizeInBytes, cntReadPtr, sizeInBytes) == 0, L"Cannot copy memory.");
			readPtr += maxCount;
			return maxCount;
		}
		else
		{
			auto leftCount = std::min(maxCount, size_t(bufferEnd - cntReadPtr));
			auto sizeInBytes = leftCount * sizeof(Elem);
			ThrowIfNot(memcpy_s(dest, sizeInBytes, cntReadPtr, sizeInBytes) == 0, L"Cannot copy memory.");
			readPtr += leftCount;
			dest += leftCount;
			cntReadPtr += leftCount;
			maxCount -= leftCount;
			if (maxCount)
			{
				cntReadPtr = buffer.get();
				auto rightCount = std::min(maxCount, size_t(cntWritePtr - buffer.get()));
				sizeInBytes = rightCount * sizeof(Elem);
				ThrowIfNot(memcpy_s(dest, sizeInBytes, cntReadPtr, sizeInBytes) == 0, L"Cannot copy memory.");
				readPtr = cntReadPtr + rightCount;
				leftCount += rightCount;
			}
			if (writePtr != readPtr)
				bufferFull = false;
			return leftCount;
		}
	}

	size_t tell_not_get() const noexcept
	{
		auto cntReadPtr = readPtr, cntWritePtr = writePtr;
		if (cntReadPtr <= cntWritePtr && !bufferFull)
			return size_t(cntWritePtr - cntReadPtr);
		else
			return size_t(bufferEnd - cntReadPtr + (cntWritePtr - buffer.get()));
	}

	size_t write(const Elem* src, size_t maxCount)
	{
		auto cntReadPtr = readPtr, cntWritePtr = writePtr;
		if (cntWritePtr == cntReadPtr && bufferFull) return 0;
		else if (cntWritePtr < cntReadPtr)
		{
			maxCount = std::min(maxCount, size_t(cntReadPtr - cntWritePtr));
			auto sizeInBytes = maxCount * sizeof(Elem);
			ThrowIfNot(memcpy_s(cntWritePtr, sizeInBytes, src, sizeInBytes) == 0, L"Cannot copy memory.");
			writePtr += maxCount;
			if (writePtr == readPtr)
				bufferFull = true;
			return maxCount;
		}
		else
		{
			auto leftCount = std::min(maxCount, size_t(bufferEnd - cntWritePtr));
			auto sizeInBytes = leftCount * sizeof(Elem);
			ThrowIfNot(memcpy_s(cntWritePtr, sizeInBytes, src, sizeInBytes) == 0, L"Cannot copy memory.");
			writePtr += leftCount;
			src += leftCount;
			cntWritePtr += leftCount;
			maxCount -= leftCount;
			if (maxCount)
			{
				cntWritePtr = buffer.get();
				auto rightCount = std::min(maxCount, size_t(cntReadPtr - buffer.get()));
				sizeInBytes = rightCount * sizeof(Elem);
				ThrowIfNot(memcpy_s(cntWritePtr, sizeInBytes, src, sizeInBytes) == 0, L"Cannot copy memory.");
				writePtr = cntWritePtr + rightCount;
				leftCount += rightCount;
			}
			if (writePtr == readPtr)
				bufferFull = true;
			return leftCount;
		}
	}

	void clear()
	{
		readPtr = writePtr = buffer.get();
		bufferFull = false;
	}
private:
	size_t bufferSize;
	bool bufferFull = false;
	std::unique_ptr<Elem[]> buffer;
	Elem* readPtr, *writePtr, *bufferEnd;
};

END_NS_CORE

#include "pop_min_max.h"