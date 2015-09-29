//
// Tomato Core
// RIFF 支持
// 作者：SunnyCase
// 创建时间：2015-09-29
//
#include "pch.h"
#include "riff.h"

using namespace NS_CORE;
using namespace NS_CORE::Riff;

RiffReader::RiffReader()
	:_currentPosition(0), _drained(true), _skipSize(0), _cacheFilled(0)
{
}

RiffReader::~RiffReader()
{
}

void RiffReader::ProvideData(const byte * buffer, size_t size)
{
	ThrowIfNot(_drained, L"Chunk isn't drained.");
	_buffer = buffer;
	_bufferSize = size;
	_bufferEaten = 0;
	_drained = false;
}

bool RiffReader::FindChunkBase(byte* chunk, size_t headerSize, size_t& position, bool skip)
{
	assert(headerSize >= sizeof(ChunkHeader));
	if (_drained) return false;

	// 1. skip
	// 1.1 skip cache
	if (_skipSize)
	{
		const auto cacheAvail = _cacheFilled;
		auto toSkip = std::min(cacheAvail, _skipSize);
		if (toSkip == cacheAvail)
			_cacheFilled = 0;
		else
		{
			ThrowIfNot(memmove_s(_cache.data(), _cache.size(), _cache.data() + toSkip, cacheAvail - toSkip) == 0, L"Cannot move memory.");
			_cacheFilled -= toSkip;
		}
		_skipSize -= toSkip;
	}
	// 1.2 skip buffer
	if (_skipSize)
	{
		const auto bufferAvail = _bufferSize - _bufferEaten;
		auto toSkip = std::min(bufferAvail, _skipSize);
		_bufferEaten += toSkip;
		_skipSize -= toSkip;
	}

	// 2. parse chunk
	bool result = false;
	// 2.1 parse chunkHeader
	auto avail = _bufferSize - _bufferEaten + _cacheFilled;
	// 2.1.1 不够读取，入 cache
	if (avail < headerSize)
	{
		if (avail)
		{
			auto toCache = size_t(_bufferSize - _bufferEaten);
			auto oldSize = _cacheFilled;
			ThrowIfNot(memcpy_s(_cache.data() + oldSize, toCache, _buffer + _bufferEaten, toCache) == 0, L"Cannot copy memory.");
			_bufferEaten += toCache;
			_cacheFilled += toCache;
			assert(_cacheFilled <= _cache.size());
		}
	}
	// 2.1.2 足够读取
	else
	{
		auto dest = chunk;
		size_t eaten = 0;
		// 2.1.2.1 读取 cache
		const auto cacheAvail = _cacheFilled;
		if (cacheAvail)
		{
			ThrowIfNot(memcpy_s(dest, headerSize, _cache.data(), cacheAvail) == 0, L"Cannot copy memory.");
			dest += cacheAvail;
			eaten = cacheAvail;
			_cacheFilled = 0;
		}
		// 2.1.2.1 读取 buffer
		if (eaten < headerSize)
		{
			const auto bufferAvail = size_t(_bufferSize - _bufferEaten);
			auto toEat = headerSize - eaten;
			assert(bufferAvail >= toEat);
			ThrowIfNot(memcpy_s(dest, toEat, _buffer + _bufferEaten, toEat) == 0, L"Cannot copy memory.");
			_bufferEaten += toEat;
		}
		position = _currentPosition + headerSize;
		const auto dataSize = reinterpret_cast<ChunkHeader*>(chunk)->Size;
		const auto padding = dataSize % 2;
		const auto skipSize = skip ? dataSize + padding : 0;
		_currentPosition += skipSize + headerSize;
		_skipSize = skipSize;
		result = true;
	}
	assert(_bufferEaten <= _bufferSize);
	if (_bufferEaten == _bufferSize)
		_drained = true;
	return result;
}

bool RiffReader::FindChunk(Chunk & chunk)
{
	auto result = FindChunkBase(reinterpret_cast<byte*>(&chunk), sizeof(ChunkHeader), chunk.Position);
	ThrowIfNot(chunk.Header.Id != ChunkHeaders::Riff, L"Should call FindRiffChunk.");
	return result;
}

bool RiffReader::FindRiffChunk(RiffChunkHeader& chunk)
{
	size_t position;
	auto result = FindChunkBase(reinterpret_cast<byte*>(&chunk), sizeof(RiffChunkHeader), position, false);
	ThrowIfNot(chunk.Id == ChunkHeaders::Riff, L"Should call FindChunk.");
	return result;
}