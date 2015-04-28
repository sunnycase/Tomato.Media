//
// Tomato Media
// ID3v2 附加信息
// 
// (c) SunnyCase 
// 创建日期 2015-04-03
#include "pch.h"
#include "id3v2.h"
#include "Utilities/encoding.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace concurrency;
#ifdef __cplusplus_winrt
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
#endif

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

static concurrency::task<bool> ReadExtendedHeader(DataReader^ reader)
{
	return create_task(reader->LoadAsync(EXTHEADER_LEN))
		.then([=](size_t read)
	{
		if (read < EXTHEADER_LEN)
			return task_from_result(false);
		ID3V2ExtendedHeader extHeader;
		reader->ReadBytes(ArrayReference<byte>(reinterpret_cast<byte*>(&extHeader), sizeof(extHeader)));
		// 读取 CRC
		if (extHeader.Flags & ID3V2ExtendedHeader::HasCRC)
			return create_task(reader->LoadAsync(CRC_LEN))
			.then([=](size_t read)
		{
			if (read < CRC_LEN)
				return false;
			auto crc = reader->ReadUInt32();
			return true;
		});
		return task_from_result(true);
	});
}

task<bool> ID3V2Meta::Read(IMediaSourceIntern* source, std::function<bool(const ID3V2FrameKind&)> framePredicate)
{
	this->valid = false;
#ifdef __cplusplus_winrt
	auto stream = source->CreateRTRandomAccessStream();
	if (stream->Size < HEADER_LEN)
		return task_from_result(false);
	// 转到头部并读取 Header
	stream->Seek(0);
	auto reader = ref new DataReader(stream);
	return create_task(reader->LoadAsync(HEADER_LEN))
		.then([=](size_t)
	{
		ID3V2Header header;
		reader->ReadBytes(ArrayReference<byte>(reinterpret_cast<byte*>(&header), sizeof(header)));

		// 检测头部
		if (header.Id != GoodId)
			return task_from_result(false);
		// 检测版本号
		if (header.Version != V3VERSION &&
			header.Version != V4VERSION)
			return task_from_result(false);

		header.FixSize();
		this->reversion = header.Reversion;
		this->flags = header.Flags;
		this->restLength = header.Size;

		auto readExtHeaderTask = task_from_result(true);
		// 读取 Extended Header
		if ((header.Flags & ID3V2Flags::ExtendedHeader) == ID3V2Flags::ExtendedHeader)
			readExtHeaderTask = ReadExtendedHeader(reader);
		return readExtHeaderTask.then([=](bool valid)
		{
			if (!valid)
				return task_from_result(false);
			// 读取帧
			return create_task(reader->LoadAsync(this->restLength))
				.then([=](size_t read)
			{
				auto buffer = ref new Array<byte>(read);
				reader->ReadBytes(buffer);
				BinaryReader br(buffer->Data, buffer->Length);
				try
				{
					while (br.GetAvailable() && ReadFrame(br, framePredicate));
				}
				catch(...)
				{ }

				this->valid = true;
				return true;
			});
		});
	});
#endif
}

task<bool> ID3V2Meta::ReadBriefMetadata(IMediaSourceIntern * source, std::shared_ptr<MediaMetadataContainer> container)
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

	return meta->Read(source, [&](const ID3V2FrameKind& kind)
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
				cntner->Add<DefaultMediaMetadatas::TrackNumber>(_wtoi(frame->GetText().c_str()));
			if (frame = pmeta->GetSingleTextFrame(ID3V2FrameKinds::TCON))
				cntner->Add<DefaultMediaMetadatas::Genre>(frame->GetText());
		}
		return good;
	});
}

task<bool> ID3V2Meta::ReadExtraMetadata(IMediaSourceIntern * source, std::shared_ptr<MediaMetadataContainer> container)
{
	static std::set<ID3V2FrameKind> extraKinds =
	{
		ID3V2FrameKinds::TXXX
	};

	auto meta = std::make_shared<ID3V2Meta>();

	return meta->Read(source, [&](const ID3V2FrameKind& kind)
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

bool ID3V2Meta::ReadFrame(BinaryReader& reader, const std::function<bool(const ID3V2FrameKind&)>& framePredicate)
{
	std::unique_ptr<ID3V2Frame> frame;
	if (ID3V2Frame::ReadFrame(reader, framePredicate, frame) == ID3V2FrameKinds::Padding)
		return false;
	if (frame)
		frames.emplace(frame->GetKind(), std::move(frame));
	return true;
}