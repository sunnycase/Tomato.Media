//
// Tomato Media
// 媒体元数据
// 
// 作者：SunnyCase 
// 创建时间：2015-03-18
#pragma once
#include <Tomato.Media/Tomato.Media.h>
#include <Tomato.Core/any.h>
#include <unordered_map>
#include <string>

DEFINE_NS_MEDIA

#define DECLARE_MEDIAMETA_TYPE(name, valuetype) struct name { \
TOMATO_MEDIA_API static const std::wstring Name; \
using value_type = valuetype;};

struct Picture
{
	std::wstring MimeType;
	std::wstring Description;
	std::wstring Type;
	std::vector<byte> Data;
};

class DefaultMediaMetadatas
{
public:
	// 标题
	DECLARE_MEDIAMETA_TYPE(Title, std::wstring);
	// 专辑
	DECLARE_MEDIAMETA_TYPE(Album, std::wstring);
	// 专辑艺术家
	DECLARE_MEDIAMETA_TYPE(AlbumArtist, std::wstring);
	// 艺术家
	DECLARE_MEDIAMETA_TYPE(Artist, std::wstring);
	// 年代
	DECLARE_MEDIAMETA_TYPE(Year, uint32_t);
	// 音轨编号
	DECLARE_MEDIAMETA_TYPE(TrackNumber, std::wstring);
	// 流派
	DECLARE_MEDIAMETA_TYPE(Genre, std::wstring);
	// 歌词
	DECLARE_MEDIAMETA_TYPE(Lyrics, std::wstring);
	// 图片
	DECLARE_MEDIAMETA_TYPE(Picture, NS_MEDIA::Picture);
};

// 媒体元数据容器
class MediaMetadataContainer
{
public:
	MediaMetadataContainer()
	{

	}

	// 添加元数据
	template<typename TMeta>
	void Add(typename TMeta::value_type value)
	{
		metadatas.emplace(TMeta::Name, Core::any(value));
	}

	template<typename TMeta>
	// 获取元数据
	typename const TMeta::value_type& Get() const
	{
		return any_cast<typename TMeta::value_type>(metadatas.find(TMeta::Name)->second);
	}

	template<typename TMeta>
	// 元数据是否存在
	bool Exists() const
	{
		return metadatas.find(TMeta::Name) != metadatas.end();
	}

	template<typename TMeta>
	// 获取元数据
	typename const TMeta::value_type& GetOrDefault(typename TMeta::value_type& defaultValue = typename TMeta::value_type()) const
	{
		if (Exists<TMeta>())
			return Get<TMeta>();
		return defaultValue;
	}

	template<typename TMeta, typename TCallback>
	// 获取元数据
	void ForEach(TCallback&& callback) const
	{
		auto range = metadatas.equal_range(TMeta::Name);
		for (auto it = range.first;it != range.second;++it)
			if (callback(any_cast<typename TMeta::value_type>(it->second)))
				break;
	}

	size_t GetSize() const noexcept
	{
		return metadatas.size();
	}
private:
	std::unordered_multimap<std::wstring, Core::any> metadatas;
};

TOMATO_MEDIA_API concurrency::task<void> TryGetMediaMetadata(MediaMetadataContainer& container, IMFByteStream* byteStream, const std::wstring& uriHint, bool brief = true);

END_NS_MEDIA