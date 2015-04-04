//
// Tomato Media
// [Internal] 二进制读取器
// 
// (c) SunnyCase 
// 创建日期 2015-04-04
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

enum class SeekOrigin
{
	Begin,
	Current,
	End
};

// 二进制读取器
class BinaryReader
{
public:
	BinaryReader(byte* buffer, size_t length)
		:buffer(buffer), currentPos(buffer), length(length)
	{
		assert(buffer);
	}

	byte* GetCurrentPointer() const noexcept
	{
		return currentPos;
	}

	int64_t GetPosition() const noexcept
	{
		return currentPos - buffer;
	}

	size_t GetAvailable() const noexcept
	{
		return length - size_t(GetPosition());
	}

	void Seek(int64_t offset, SeekOrigin origin)
	{
		switch (origin)
		{
		case SeekOrigin::Begin:
			THROW_IF_NOT(offset > 0 && size_t(offset) <= length, "超出了流的长度。");
			currentPos = buffer + offset;
			break;
		case SeekOrigin::Current:
		{
			auto new_pos = GetPosition() + offset;
			Seek(new_pos, SeekOrigin::Begin);
		}
		break;
		case SeekOrigin::End:
		{
			auto new_pos = int64_t(length) + offset;
			Seek(new_pos, SeekOrigin::Begin);
		}
		break;
		}
	}

	void Read(byte* buffer, size_t count)
	{
		memcpy_s(buffer, count, Step(count), count);
	}

	byte ReadByte()
	{
		return *Step(1);
	}

	uint16_t ReadUInt16()
	{
		return *reinterpret_cast<const uint16_t*>(Step(2));
	}

	int32_t ReadInt32()
	{
		return *reinterpret_cast<const int32_t*>(Step(4));
	}

	uint32_t ReadUInt32()
	{
		return *reinterpret_cast<const uint32_t*>(Step(4));
	}

	std::string ReadString()
	{
		std::vector<char> bytes;
		byte b = 0;
		while (b = *Step(1))
			bytes.emplace_back((char)b);
		return std::string(bytes.begin(), bytes.end());
	}

	template<size_t n>
	std::string ReadString()
	{
		std::string str;
		str.resize(n);

		Read((byte*)str.data(), n);
		return str;
	}

	std::string ReadString(size_t n)
	{
		std::string str;
		str.resize(n);

		Read((byte*)str.data(), n);
		return str;
	}

	template<size_t n>
	std::array<byte, n> Read()
	{
		std::array<byte, n> bytes;

		Read(bytes);
		return bytes;
	}

	template<size_t n>
	void Read(std::array<byte, n>& arr)
	{
		Read(arr.data(), n);
	}

	std::unique_ptr<byte[]> Read(size_t count)
	{
		auto buffer = std::make_unique<byte[]>(count);

		Read(buffer.get(), count);
		return buffer;
	}

	template<typename T, typename = std::enable_if_t<std::is_trivial<T>::value>>
	T Read()
	{
		T value;
		Read(reinterpret_cast<byte*>(&value), sizeof(T));
		return value;
	}

	uint32_t ReadUInt32BE()
	{
		return to_be(ReadUInt32());
	}
private:
	byte* Step(int64_t offset)
	{
		auto ptr = currentPos;
		Seek(offset, SeekOrigin::Current);
		return ptr;
	}
private:
	byte* buffer;
	byte* currentPos;
	size_t length;
};

NSED_TOMATO_MEDIA