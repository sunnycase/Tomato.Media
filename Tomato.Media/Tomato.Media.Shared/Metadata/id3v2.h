//
// Tomato Media
// ID3v2 附加信息
// 
// 作者：SunnyCase 
// 创建时间：2015-04-03
#pragma once
#include "MediaMetadata.h"
#include "ID3V2Frame.h"
#include <ppltasks.h>
#include <mfidl.h>

DEFINE_NS_MEDIA_INTERN

///<summary>ID3v2 标志</summary>
enum class ID3V2Flags : byte
{
	// Unsynchronisation
	Unsynchronisation = 1 << 7,
	// 扩展头部
	ExtendedHeader = 1 << 6,
	// 测试用途
	Experimental = 1 << 5
};

DEFINE_ENUM_FLAG_OPERATORS(ID3V2Flags);

///<summary>ID3v2.3 附加信息</summary>
class ID3V2Meta
{
public:
	ID3V2Meta();

	///<summary>是否读取成功</summary>
	bool IsGood() const noexcept;
	///<summary>读取附加信息</summary>
	///<param name="framePredicate">允许的帧类型</param>
	///<return>是否读取成功</return>
	concurrency::task<bool> Read(IMFByteStream* byteStream, const std::function<bool(const ID3V2FrameKind&)>& framePredicate = [](const ID3V2FrameKind&) {return true; });
	///<summary>读取简明附加信息</summary>
	static concurrency::task<bool> ReadBriefMetadata(IMFByteStream* byteStream, std::shared_ptr<MediaMetadataContainer> container);
	///<summary>读取额外附加信息</summary>
	static concurrency::task<bool> ReadExtraMetadata(IMFByteStream* byteStream, std::shared_ptr<MediaMetadataContainer> container);

	template<class T>
	const T* GetFrame(const ID3V2FrameKind& kind) const noexcept
	{
		static_assert(std::is_base_of<ID3V2Frame, T>::value, "T 必须是帧类型。");

		auto it = frames.find(kind);
		if (it != frames.end())
			return (T*)it->second.get();
		return nullptr;
	}

	template<class T>
	void ForEachFrame(const ID3V2FrameKind& kind, std::function<bool(const T*)> handler) const
	{
		static_assert(std::is_base_of<ID3V2Frame, T>::value, "T 必须是帧类型。");

		auto matches = frames.equal_range(kind);
		for (auto it = matches.first; it != matches.second; ++it)
		{
			if (handler((T*)it->second.get()))
				break;
		}
	}

	template<class T>
	void ForEachFrame(const ID3V2FrameKind& kind, std::function<bool(T*)> handler)
	{
		static_assert(std::is_base_of<ID3V2Frame, T>::value, "T 必须是帧类型。");

		auto matches = frames.equal_range(kind);
		for (auto it = matches.first; it != matches.second; ++it)
		{
			if (handler((T*)it->second.get()))
				break;
		}
	}

	const ID3V2FrameSingleText* GetSingleTextFrame(const ID3V2FrameKind& kind) const noexcept
	{
		return GetFrame<ID3V2FrameSingleText>(kind);
	}

	std::wstring GetTextOrEmpty(const ID3V2FrameKind& kind)
	{
		auto frame = GetSingleTextFrame(kind);
		if (frame)
			return frame->GetText();
		return std::wstring();
	}
private:
	///<summary>读取帧</summary>
	///<return>是否继续</return>
	bool ReadFrame(Core::BinaryReader<byte*>& reader, const std::function<bool(const ID3V2FrameKind&)>& framePredicate);
private:
	bool valid;						// 读取成功
	byte reversion;					// 修订版本号
	ID3V2Flags flags;				// 标志
	uint32_t restLength;			// 头部以外长度
	std::unordered_multimap<ID3V2FrameKind, std::unique_ptr<ID3V2Frame>> frames;	// 帧
};

END_NS_MEDIA_INTERN