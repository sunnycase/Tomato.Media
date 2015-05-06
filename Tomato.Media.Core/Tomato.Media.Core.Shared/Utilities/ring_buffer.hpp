//
// Tomato Media
// 环形缓冲
// 
// (c) SunnyCase 
// 创建日期 2015-05-01
#pragma once
#include "../include/platform.h"
#include "memory.h"

namespace Tomato
{
	template<typename Elem>
	class ring_buffer
	{
	public:
		ring_buffer(){}

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
				MemoryCopy(dest, sizeInBytes, cntReadPtr, sizeInBytes);
				readPtr += maxCount;
				return maxCount;
			}
			else
			{
				auto leftCount = std::min(maxCount, size_t(bufferEnd - cntReadPtr));
				auto sizeInBytes = leftCount * sizeof(Elem);
				MemoryCopy(dest, sizeInBytes, cntReadPtr, sizeInBytes);
				readPtr += leftCount;
				dest += leftCount;
				cntReadPtr += leftCount;
				maxCount -= leftCount;
				if (maxCount)
				{
					cntReadPtr = buffer.get();
					auto rightCount = std::min(maxCount, size_t(cntWritePtr - buffer.get()));
					sizeInBytes = rightCount * sizeof(Elem);
					MemoryCopy(dest, sizeInBytes, cntReadPtr, sizeInBytes);
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
				MemoryCopy(cntWritePtr, sizeInBytes, src, sizeInBytes);
				writePtr += maxCount;
				if (writePtr == readPtr)
					bufferFull = true;
				return maxCount;
			}
			else
			{
				auto leftCount = std::min(maxCount, size_t(bufferEnd - cntWritePtr));
				auto sizeInBytes = leftCount * sizeof(Elem);
				MemoryCopy(cntWritePtr, sizeInBytes, src, sizeInBytes);
				writePtr += leftCount;
				src += leftCount;
				cntWritePtr += leftCount;
				maxCount -= leftCount;
				if (maxCount)
				{
					cntWritePtr = buffer.get();
					auto rightCount = std::min(maxCount, size_t(cntReadPtr - buffer.get()));
					sizeInBytes = rightCount * sizeof(Elem);
					MemoryCopy(cntWritePtr, sizeInBytes, src, sizeInBytes);
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
}