//
// Tomato Core
// 二进制读取器
// 
// 作者：SunnyCase 
// 创建时间：2015-04-04
#pragma once
#include "Tomato.Core.h"
#include <vector>
#include "block_buffer.hpp"

DEFINE_NS_CORE

inline uint32_t to_be(uint32_t value) noexcept
{
	return static_cast<uint32_t>(_byteswap_ulong(static_cast<unsigned long>(value)));
}

enum class SeekOrigin
{
	Begin,
	Current,
	End
};

namespace Details
{
	template<typename Provider>
	class BinaryReaderImpl
	{
	public:
		int64_t GetPosition() const noexcept
		{
			return Derived()->GetPositionCore();
		}

		size_t GetAvailable() const noexcept
		{
			return Derived()->GetAvailableCore();
		}

		void Seek(int64_t offset, SeekOrigin origin)
		{
			Derived()->SeekCore(offset, origin);
		}

		void Read(byte* buffer, size_t count)
		{
			ThrowIfNot(memcpy_s(buffer, count, Derived()->Step(count), count) == 0, L"Copy error.");
		}

		byte ReadByte()
		{
			return *Derived()->Step(1);
		}

		uint16_t ReadUInt16()
		{
			return *reinterpret_cast<const uint16_t*>(Derived()->Step(2));
		}

		int32_t ReadInt32()
		{
			return *reinterpret_cast<const int32_t*>(Derived()->Step(4));
		}

		uint32_t ReadUInt32()
		{
			return *reinterpret_cast<const uint32_t*>(Derived()->Step(4));
		}

		std::string ReadString()
		{
			std::vector<char> bytes;
			byte b = 0;
			while (b = *Derived()->Step(1))
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
		Provider* Derived() noexcept
		{
			return static_cast<Provider*>(this);
		}

		const Provider* Derived() const noexcept
		{
			return static_cast<const Provider*>(this);
		}
	};
}

template<typename TSrc>
// 二进制读取器
class BinaryReader
{};

template<>
class BinaryReader<byte*> : public Details::BinaryReaderImpl<BinaryReader<byte*>>
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

	void Read(byte* buffer, size_t count)
	{
		ThrowIfNot(memcpy_s(buffer, count, Step(count), count) == 0, L"Copy error.");
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

	template<typename T, typename = std::enable_if_t<std::is_pod<T>::value>>
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
	friend class Details::BinaryReaderImpl<BinaryReader>;

	byte* Step(int64_t offset)
	{
		auto ptr = currentPos;
		Seek(offset, SeekOrigin::Current);
		return ptr;
	}

	int64_t GetPositionCore() const noexcept
	{
		return currentPos - buffer;
	}

	size_t GetAvailableCore() const noexcept
	{
		return length - size_t(GetPosition());
	}

	void SeekCore(int64_t offset, SeekOrigin origin)
	{
		switch (origin)
		{
		case SeekOrigin::Begin:
			ThrowIfNot(offset > 0 && size_t(offset) <= length, L"Stream offset exceeded.");
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
private:
	byte* buffer;
	byte* currentPos;
	size_t length;
};

#ifdef __IMFByteStream_INTERFACE_DEFINED__

template<>
class BinaryReader<IMFByteStream> : public Details::BinaryReaderImpl<BinaryReader<IMFByteStream>>
{
public:
	BinaryReader(IMFByteStream* stream)
		:_stream(stream)
	{
		_buffer.init(4096);
	}

	uint64_t GetLength() const noexcept
	{
		QWORD length;
		ThrowIfFailed(_stream->GetLength(&length));
		return uint64_t(length);
	}

	size_t Load(size_t size)
	{
		ULONG read;
		auto buffer = std::make_unique<byte[]>(size);
		ThrowIfFailed(_stream->Read(buffer.get(), size, &read));
		_buffer.write(buffer.get(), read);
		return size;
	}
private:
	friend class Details::BinaryReaderImpl<BinaryReader>;

	byte* Step(size_t offset)
	{
		_cache = std::make_unique<byte[]>(offset);
		_buffer.read(_cache.get(), offset);
		return _cache.get();
	}

	uint64_t GetPositionCore() const noexcept
	{
		QWORD position;
		ThrowIfFailed(_stream->GetCurrentPosition(&position));
		return position - _buffer.tell_not_get();
	}

	uint64_t GetAvailableCore() const noexcept
	{
		return _buffer.tell_not_get();
	}

	void SeekCore(int64_t offset, SeekOrigin origin)
	{
		QWORD after;
		switch (origin)
		{
		case SeekOrigin::Begin:
			_buffer.clear();
			ThrowIfFailed(_stream->Seek(msoBegin, offset, 0, &after));
			break;
		case SeekOrigin::Current:
		{
			auto new_pos = GetPosition() + offset;
			Seek(new_pos, SeekOrigin::Begin);
		}
		break;
		case SeekOrigin::End:
		{
			auto new_pos = int64_t(GetLength()) + offset;
			Seek(new_pos, SeekOrigin::Begin);
		}
		break;
		}
	}
private:
	WRL::ComPtr<IMFByteStream> _stream;
	block_buffer<byte> _buffer;
	std::unique_ptr<byte[]> _cache;
};

#endif

END_NS_CORE