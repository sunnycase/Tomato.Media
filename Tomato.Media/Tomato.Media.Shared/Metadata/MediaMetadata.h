//
// Tomato Media
// 媒体元数据
// 
// 作者：SunnyCase 
// 创建时间：2015-03-18
#pragma once
#include "common.h"
#include "../../include/any.h"
#include <unordered_map>

DEFINE_NS_MEDIA_INTERN

#define DECLARE_MEDIAMETA_TYPE(name, valuetype) struct name { \
static const std::wstring Name; \
using value_type = valuetype;};

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

	size_t GetSize() const noexcept
	{
		return metadatas.size();
	}
private:
	std::unordered_multimap<std::wstring, Core::any> metadatas;
};

concurrency::task<std::shared_ptr<MediaMetadataContainer>> GetMediaMetadata(IMFByteStream* byteStream, bool brief = true);

END_NS_MEDIA_INTERN