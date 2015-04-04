//
// Tomato Music 媒体
// ID3v1 附加信息
// 
// (c) SunnyCase 
// 创建日期 2014-10-12
#include "pch.h"
#include "id3v1.h"
#include "Utilities/encoding.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace concurrency;
#ifdef __cplusplus_winrt
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
#endif

enum
{
	// 总长度
	TOTAL_LEN = 128,
	// 头部长度
	HEADER_LEN = 3,
	// 标题长度
	TITLE_LEN = 30,
	// 艺术家长度
	ARTIST_LEN = 30,
	// 专辑长度
	ALBUM_LEN = 30,
	// 注释（和音轨）长度
	COMMENT_AND_TRACK_LEN = 30,
	// 音轨前的 Padding 位置
	TRACK_PAD_OFFSET = 28,
	// 音轨的位置
	TRACK_OFFSET = 29,
	// 流派长度
	GENRE_LEN = 1
};

static const std::array<byte, HEADER_LEN> GoodHeader = { 'T', 'A', 'G' };

#pragma pack(push, 1)
struct ID3V1Tag
{
	std::array<byte, HEADER_LEN> Header;
	char Title[30];
	char Artist[30];
	char Album[30];
	uint32_t Year;
	union
	{
		char Comment[30];
		struct
		{
			char LessComment[28];
			bool NoTrackNumber;
			byte TrackNumber;
		};
	};
	byte Genre;
};
#pragma pack(pop)

ID3V1Meta::ID3V1Meta()
	:valid(false)
{

}

bool ID3V1Meta::IsGood() const noexcept
{
	return valid;
}

task<bool> ID3V1Meta::ReadMetadata(IMediaSourceIntern* source, std::shared_ptr<MediaMetadataContainer> container)
{
	auto meta = std::make_shared<ID3V1Meta>();

	return meta->Read(source).then([=](bool good)
	{
		if (good)
		{
			auto pmeta = meta.get();
			auto cntner = container.get();
			cntner->Add<DefaultMediaMetadatas::Title>(pmeta->title);
			cntner->Add<DefaultMediaMetadatas::Album>(pmeta->album);
			cntner->Add<DefaultMediaMetadatas::Artist>(pmeta->artist);
			cntner->Add<DefaultMediaMetadatas::Year>(pmeta->year);
			cntner->Add<DefaultMediaMetadatas::TrackNumber>(pmeta->track);
			cntner->Add<DefaultMediaMetadatas::Genre>(pmeta->genre.ToString()->Data());
		}
		return good;
	});
}

task<bool> ID3V1Meta::Read(IMediaSourceIntern* source)
{
	this->valid = false;
#ifdef __cplusplus_winrt
	auto stream = source->CreateRTRandomAccessStream();
	if (stream->Size < TOTAL_LEN)
		return task_from_result(false);
	// 转到尾部并读取总长度那么多字节
	stream->Seek(stream->Size - TOTAL_LEN);
	auto reader = ref new DataReader(stream);
	return create_task(reader->LoadAsync(TOTAL_LEN))
		.then([=](size_t)
	{
		ID3V1Tag tag;
		reader->ReadBytes(ArrayReference<byte>(reinterpret_cast<byte*>(&tag), sizeof(tag)));
		// 检测头部
		if (tag.Header != GoodHeader)
			return false;
		this->title = s2ws(tag.Title, CP_ACP);
		this->artist = s2ws(tag.Artist, CP_ACP);
		this->album = s2ws(tag.Album, CP_ACP);
		this->year = tag.Year;
		if (tag.NoTrackNumber)
		{
			this->comment = s2ws(tag.Comment, CP_ACP);
			this->track = 0;
		}
		else
		{
			this->comment = s2ws(tag.LessComment, CP_ACP);
			this->track = tag.TrackNumber;
		}
		this->genre = static_cast<ID3v1Genre>(
			tag.Genre < (byte)ID3v1Genre::COUNT ? tag.Genre : (byte)ID3v1Genre::Unknown);

		this->valid = true;
		return true;
	});
#endif
}