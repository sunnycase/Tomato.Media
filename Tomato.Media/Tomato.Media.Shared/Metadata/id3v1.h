//
// Tomato Media
// ID3v1 附加信息
// 
// 作者：SunnyCase 
// 创建时间：2015-04-03
#pragma once
#include <Tomato.Media/MediaMetadata.h>
#include <ppltasks.h>
#include <mfidl.h>

DEFINE_NS_MEDIA

///<summary>ID3v1 流派</summary>
enum class ID3v1Genre
{
	Blues,
	ClassicRock,
	Country,
	Dance,
	Disco,
	Funk,
	COUNT,
	Unknown = 255
};

std::wstring to_string(ID3v1Genre value);

///<summary>ID3v1 附加信息</summary>
class ID3V1Meta
{
public:
	ID3V1Meta();

	///<summary>是否读取成功</summary>
	bool IsGood() const noexcept;
	///<summary>读取附加信息</summary>
	///<return>是否读取成功</return>
	concurrency::task<bool> Read(IMFByteStream* byteStream);
	///<summary>读取附加信息</summary>
	static concurrency::task<bool> ReadMetadata(IMFByteStream* byteStream, MediaMetadataContainer& container);
private:
	bool valid;						// 读取成功
	std::wstring title;				// 标题
	std::wstring artist;			// 艺术家
	std::wstring album;				// 专辑
	uint32_t year;					// 年代
	std::wstring comment;			// 注释
	byte track;						// 音轨
	ID3v1Genre genre;				// 流派
};

END_NS_MEDIA