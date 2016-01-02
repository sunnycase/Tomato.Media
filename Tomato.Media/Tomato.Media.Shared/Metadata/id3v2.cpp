//
// Tomato Media
// ID3v2 附加信息
// 
// 作者：SunnyCase 
// 创建时间：2015-04-03
#include "pch.h"
#include "id3v2.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace NS_MEDIA_INTERN;
using namespace concurrency;

namespace
{
	enum
	{
		// 头部长度
		HEADER_LEN = 10,
		// Id 长度
		ID_LEN = 3,
		// 版本
		VERSION_LEN = 1,
		// 修订版本号
		REVERSION_LEN = 1,
		// 标志
		FLAG_LEN = 1,
		// 头部以外长度
		RESTLEN_LEN = 4,
		// 扩展头部长度
		EXTHEADER_LEN = 10,
		// CRC 长度
		CRC_LEN = 4
	};

#pragma pack(push, 1)
	struct ID3V2Header
	{
		std::array<byte, ID_LEN> Id;
		byte Version;
		byte Reversion;
		ID3V2Flags Flags;
		uint32_t Size;

		void FixSize()
		{
			Size = to_be(Size);
			Size = ((Size & 0x7F000000) >> 3) |
				((Size & 0x7F0000) >> 2) |
				((Size & 0x7F00) >> 1) |
				(Size & 0x7F);
		}
	};

	struct ID3V2ExtendedHeader
	{
		uint32_t Size;
		uint16_t Flags;
		uint32_t PaddingSize;

		enum
		{
			HasCRC = 1 << 15
		};
	};

	static_assert(sizeof(ID3V2Header) == HEADER_LEN, "sizeof ID3V2Header must equal to HEADER_LEN.");
	static_assert(sizeof(ID3V2ExtendedHeader) == EXTHEADER_LEN, "sizeof ID3V2ExtendedHeader must equal to EXTHEADER_LEN.");
#pragma pack(pop)

	static const std::array<byte, ID_LEN> GoodId = { 'I', 'D', '3' };
	static const byte V3VERSION = 3;
	static const byte V4VERSION = 4;
}

ID3V2Meta::ID3V2Meta()
	:valid(false)
{

}

bool ID3V2Meta::IsGood() const noexcept
{
	return valid;
}

bool ReadExtendedHeader(BinaryReader<IMFByteStream>& reader)
{
	if (reader.Load(EXTHEADER_LEN) != EXTHEADER_LEN)
		return false;

	auto extHeader = reader.Read<ID3V2ExtendedHeader>();
	// 读取 CRC
	if (extHeader.Flags & ID3V2ExtendedHeader::HasCRC)
	{
		if (reader.Load(CRC_LEN) != CRC_LEN)
			return false;
		auto crc = reader.ReadUInt32();
	}
	return true;
}

task<bool> ID3V2Meta::Read(IMFByteStream* byteStream, const std::function<bool(const ID3V2FrameKind&)>& framePredicate)
{
	this->valid = false;

	BinaryReader<IMFByteStream> streamReader(byteStream);
	auto ret = [&]
	{
		if (streamReader.GetLength() < HEADER_LEN)
			return false;
		// 转到头部并读取 Header
		streamReader.Seek(0, SeekOrigin::Begin);
		if (streamReader.Load(HEADER_LEN) != HEADER_LEN)
			return false;

		auto header = streamReader.Read<ID3V2Header>();
		// 检测头部
		if (header.Id != GoodId)
			return false;
		// 检测版本号
		if (header.Version != V3VERSION &&
			header.Version != V4VERSION)
			return false;

		header.FixSize();
		this->reversion = header.Reversion;
		this->flags = header.Flags;
		this->restLength = header.Size;

		auto readExtHeaderTask = task_from_result(true);
		// 读取 Extended Header
		if ((header.Flags & ID3V2Flags::ExtendedHeader) == ID3V2Flags::ExtendedHeader
			&& !ReadExtendedHeader(streamReader))
			return false;
		auto read = streamReader.Load(this->restLength);
		auto buffer = streamReader.Read(read);
		BinaryReader<byte*> br(buffer.get(), read);
		try
		{
			while (br.GetAvailable() && ReadFrame(br, framePredicate));
		}
		catch (...)
		{
		}

		this->valid = true;
		return true;
	}();
	return task_from_result(ret);
}

task<bool> ID3V2Meta::ReadBriefMetadata(IMFByteStream* byteStream, std::shared_ptr<MediaMetadataContainer> container)
{
	static std::set<ID3V2FrameKind> briefKinds =
	{
		ID3V2FrameKinds::TCON,
		ID3V2FrameKinds::TIT2,
		ID3V2FrameKinds::TPE1,
		ID3V2FrameKinds::TPE2,
		ID3V2FrameKinds::TALB,
		ID3V2FrameKinds::TYER,
		ID3V2FrameKinds::TRCK
	};

	auto meta = std::make_shared<ID3V2Meta>();

	return meta->Read(byteStream, [&](const ID3V2FrameKind& kind)
	{
		return briefKinds.find(kind) != briefKinds.end();
	}).then([=](bool good)
	{
		if (good)
		{
			auto pmeta = meta.get();
			auto cntner = container.get();
			const ID3V2FrameSingleText* frame = nullptr;

			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TIT2))
				cntner->Add<DefaultMediaMetadatas::Title>(frame->GetText());
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TALB))
				cntner->Add<DefaultMediaMetadatas::Album>(frame->GetText());
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TPE1))
				cntner->Add<DefaultMediaMetadatas::Artist>(frame->GetText());
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TPE2))
				cntner->Add<DefaultMediaMetadatas::AlbumArtist>(frame->GetText());
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TYER))
				cntner->Add<DefaultMediaMetadatas::Year>(_wtoi(frame->GetText().c_str()));
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TRCK))
				cntner->Add<DefaultMediaMetadatas::TrackNumber>(frame->GetText());
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TCON))
				cntner->Add<DefaultMediaMetadatas::Genre>(frame->GetText());
		}
		return good;
	});
}

task<bool> ID3V2Meta::ReadExtraMetadata(IMFByteStream * byteStream, std::shared_ptr<MediaMetadataContainer> container)
{
	static std::set<ID3V2FrameKind> extraKinds =
	{
		ID3V2FrameKinds::TXXX
	};

	auto meta = std::make_shared<ID3V2Meta>();

	return meta->Read(byteStream, [&](const ID3V2FrameKind& kind)
	{
		return extraKinds.find(kind) != extraKinds.end();
	}).then([=](bool good)
	{
		if (good)
		{
			auto pmeta = meta.get();
			auto cntner = container.get();
			const ID3V2FrameTXXX* txxxFrame = nullptr;

			if ((txxxFrame = pmeta->GetFrame<ID3V2FrameTXXX>(ID3V2FrameKinds::TXXX)) &&
				txxxFrame->GetDescription() == L"LYRICS")
				cntner->Add<DefaultMediaMetadatas::Lyrics>(txxxFrame->GetValue());
		}
		return good;
	});
}

bool ID3V2Meta::ReadFrame(BinaryReader<byte*>& reader, const std::function<bool(const ID3V2FrameKind&)>& framePredicate)
{
	std::unique_ptr<ID3V2Frame> frame;
	if (ID3V2Frame::ReadFrame(reader, framePredicate, frame) == ID3V2FrameKinds::Padding)
		return false;
	if (frame)
		frames.emplace(frame->GetKind(), std::move(frame));
	return true;
}