//
// Tomato Media
// ID3v2 附加信息
// 
// 作者：SunnyCase 
// 创建时间：2015-04-03
#pragma once
#include "common.h"
#include "../../include/BinaryReader.h"
#include <array>
#include <set>
#include <unordered_map>

// 帧类别的 hash 函数
namespace std
{
	template<>
	struct hash < std::array<byte, 4> >
	{
		using elem_t = std::array < byte, 4 >;

		size_t operator()(elem_t const& elem) const noexcept
		{
			return *reinterpret_cast<const uint32_t*>(elem.data());
		}
	};
}

DEFINE_NS_MEDIA_INTERN

typedef std::array<byte, 4> ID3V2FrameKind;

///<summary>ID3v2 附加信息帧标志</summary>
enum class ID3V2FrameFlags : uint16_t
{

};

///<summary>ID3v2 附加信息帧</summary>
class ID3V2Frame
{
public:
	ID3V2Frame() {}
	virtual ~ID3V2Frame() {}

	///<summary>获取类别</summary>
	virtual const ID3V2FrameKind& GetKind() const noexcept = 0;

	///<summary>读取帧</summary>
	static ID3V2FrameKind ReadFrame(Core::BinaryReader<byte*>& reader, const std::function<bool(const ID3V2FrameKind&)>& framePredicate, std::unique_ptr<ID3V2Frame>& frame);
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader) = 0;
protected:
	uint32_t contentLength = 0;
	ID3V2FrameFlags flags;
};

typedef std::function<std::unique_ptr<ID3V2Frame>()> FrameActivator;
typedef std::unordered_map<ID3V2FrameKind, FrameActivator> FrameFactory;

///<summary>ID3v2 附加信息帧类别</summary>
class ID3V2FrameKinds
{
public:
	// Padding
	static const ID3V2FrameKind Padding;
	// Unknown
	static const ID3V2FrameKind Unknown;
	// Comments
	static const ID3V2FrameKind COMM;
	// TIT2 Title/Songname/Content
	static const ID3V2FrameKind TIT2;
	// TALB Album/Movie/Show title
	static const ID3V2FrameKind TALB;
	// TCON Content type
	static const ID3V2FrameKind TCON;
	// TPOS Part of a set
	static const ID3V2FrameKind TPOS;
	// TXXX User defined text information frame
	static const ID3V2FrameKind TXXX;
	// TPE1 Lead performer(s)/Soloist(s)
	static const ID3V2FrameKind TPE1;
	// TPE2 Band/orchestra/accompaniment
	static const ID3V2FrameKind TPE2;
	// PRIV Private frame
	static const ID3V2FrameKind PRIV;
	// Publisher
	static const ID3V2FrameKind TPUB;
	// TRCK Track number/Position in set
	static const ID3V2FrameKind TRCK;
	// Year
	static const ID3V2FrameKind TYER;
	// APIC Attached picture
	static const ID3V2FrameKind APIC;

	///<summary>从帧类别创建帧对象</summary>
	static std::unique_ptr<ID3V2Frame> Create(const ID3V2FrameKind& kind);
};

// 未知帧
class ID3V2FrameUnknown : public ID3V2Frame
{
public:
	ID3V2FrameUnknown() {}

	virtual const ID3V2FrameKind& GetKind() const noexcept
	{
		return ID3V2FrameKinds::Unknown;
	}
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader);
};

// ID3v2 文本编码类型
enum class ID3V2TextEncoding : byte
{
	ISO_8859_1 = 0,
	UCS_2 = 1
};

// Comments
class ID3V2FrameCOMM : public ID3V2Frame
{
public:
	ID3V2FrameCOMM() {}

	virtual const ID3V2FrameKind& GetKind() const noexcept
	{
		return ID3V2FrameKinds::COMM;
	}
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader);
private:
	ID3V2TextEncoding encoding;
	std::array<byte, 3> language;
	std::wstring description, text;
};

// 仅由文本组成的帧
class ID3V2FrameSingleText : public ID3V2Frame
{
public:
	ID3V2FrameSingleText() {}

	const std::wstring& GetText() const noexcept { return text; }
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader);
private:
	ID3V2TextEncoding encoding;
	std::wstring text;
};

#define ID3V2BasedOnFrameSingleText(name) class ID3V2Frame##name : public ID3V2FrameSingleText \
{														   \
public:													   \
	ID3V2Frame##name() {}								   \
														   \
	virtual const ID3V2FrameKind& GetKind() const noexcept \
	{													   \
		return ID3V2FrameKinds::##name;					   \
	}													   \
};

// TIT2 Title/Songname/Content
ID3V2BasedOnFrameSingleText(TIT2);
// Track number / Position in set
ID3V2BasedOnFrameSingleText(TRCK);
// Year
ID3V2BasedOnFrameSingleText(TYER);
// TPE1 Lead performer(s)/Soloist(s)
ID3V2BasedOnFrameSingleText(TPE1);
// TPE2 Band/orchestra/accompaniment
ID3V2BasedOnFrameSingleText(TPE2);
// Publisher
ID3V2BasedOnFrameSingleText(TPUB);
// TCON Content type
ID3V2BasedOnFrameSingleText(TCON);
// TALB Album/Movie/Show title
ID3V2BasedOnFrameSingleText(TALB);

// PRIV Private frame
class ID3V2FramePRIV : public ID3V2Frame
{
public:
	ID3V2FramePRIV() {}

	const std::wstring& GetIdentifier() const noexcept { return identifier; }
	const std::vector<byte>& GetData() const noexcept { return data; }
	virtual const ID3V2FrameKind& GetKind() const noexcept { return ID3V2FrameKinds::PRIV; }
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader);
private:
	std::wstring identifier;
	std::vector<byte> data;
};

// TXXX User defined text information frame
class ID3V2FrameTXXX : public ID3V2Frame
{
public:
	ID3V2FrameTXXX() {}

	const std::wstring& GetDescription() const noexcept { return description; }
	const std::wstring& GetValue() const noexcept { return value; }
	virtual const ID3V2FrameKind& GetKind() const noexcept { return ID3V2FrameKinds::TXXX; }
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader);
private:
	ID3V2TextEncoding encoding;
	std::wstring description, value;
};

enum class ID3V2PictureType : byte
{
	Other,
	_32x32FileIcon,
	OtherFileIcon,
	Cover_Front,
	Cover_Back,
	LeafletPage,
	Media,
	LeadArtist,
	Artist,
	Conductor,
	Band,
	Composer,
	Lyricist,
	RecordingLocation,
	DuringRecording,
	DuringPerformance,
	VideoScreenCapture,
	ABrightColouredFish,
	Illustration,
	ArtistLogotype,
	StudioLogotype,
	COUNT
};

// APIC Attached picture
class ID3V2FrameAPIC : public ID3V2Frame
{
public:
	ID3V2FrameAPIC() {}

	const std::wstring& GetMimeType() const noexcept { return mimeType; }
	const std::wstring& GetDescription() const noexcept { return description; }
	ID3V2PictureType GetPictureType() const noexcept { return pictureType; }
	const std::vector<byte>& GetData() const noexcept { return data; }
	std::vector<byte>& GetData() noexcept { return data; }
	virtual const ID3V2FrameKind& GetKind() const noexcept { return ID3V2FrameKinds::APIC; }
protected:
	virtual void ReadContent(Core::BinaryReader<byte*>&& reader);
private:
	ID3V2TextEncoding encoding;
	std::wstring mimeType, description;
	ID3V2PictureType pictureType;
	std::vector<byte> data;
};

END_NS_MEDIA_INTERN