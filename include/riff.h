//
// Tomato Core
// RIFF 支持
// 作者：SunnyCase
// 创建时间：2015-09-29
//
#pragma once
#include "common.h"
#include <mfapi.h>
#include <array>
#include <vector>

DEFINE_NS_CORE

namespace Riff
{
	enum class ChunkHeaders : DWORD
	{
		Riff = FCC('RIFF'),
		Format = FCC('fmt '),
		Data = FCC('data')
	};

	enum class FileTypes : DWORD
	{
		Wave = FCC('WAVE')
	};

#pragma pack(push, 1)
	struct ChunkHeader
	{
		ChunkHeaders Id;
		DWORD Size;
	};

	struct RiffChunkHeader : public ChunkHeader
	{
		FileTypes FileType;
	};
#pragma pack(pop)
	static_assert(sizeof(ChunkHeader) == 8, "sizeof ChunkHeader must be 8.");
	static_assert(sizeof(RiffChunkHeader) == 12, "sizeof ChunkHeader must be 12.");

	struct Chunk
	{
		ChunkHeader Header;
		size_t Position;
	};

	class TOMATO_CORE_API RiffReader
	{
	public:
		RiffReader();
		~RiffReader();

		// 1. 在 FindChunk 返回 false 前，buffer 必须有效
		// 2. FindChunk 返回 false 时才能调用。
		void ProvideData(const byte* buffer, size_t size);
		bool FindRiffChunk(RiffChunkHeader& chunk);
		bool FindChunk(Chunk& chunk);
	private:
		bool FindChunkBase(byte* chunk, size_t headerSize, size_t& position, bool skip = true);
	private:
		size_t _currentPosition;
		bool _drained;
		const byte * _buffer;
		size_t _bufferSize;
		size_t _bufferEaten;
		size_t _skipSize;
		std::array<byte, sizeof(RiffChunkHeader)> _cache;
		size_t _cacheFilled;
	};
}

END_NS_CORE