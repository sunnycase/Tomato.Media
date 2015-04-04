//
// Tomato Media
// 媒体元数据
// 
// (c) SunnyCase 
// 创建日期 2015-03-18
#pragma once
#include "platform.h"
#include <string>
#include "any.h"
#include <unordered_map>

NSDEF_TOMATO_MEDIA

#pragma warning(push)
#pragma warning(disable:4251)

// 媒体元数据
class MEDIA_CORE_API MediaMetadata
{
public:
	MediaMetadata(const std::wstring& name, const any& value)
		:name(name), value(value)
	{

	}

	MediaMetadata(const std::wstring& name, any&& value)
		:name(name), value(std::move(value))
	{

	}

	// 获取名称
	const std::wstring& GetName() const noexcept { return name; }
	// 获取值
	const any& GetValue() const noexcept { return value; }
	template<typename T>
	// 获取值
	T& GetValue() const { return value.get<T>(); }

	// 设置值
	void SetValue(const any& value) { this->value = value; }
	// 设置值
	void SetValue(any&& value) { this->value = std::move(value); }
private:
	std::wstring name;
	any value;
};

#define DECLARE_MEDIAMETA_TYPE(name, valuetype) struct name { \
MEDIA_CORE_API static const std::wstring Name; \
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
	DECLARE_MEDIAMETA_TYPE(TrackNumber, uint32_t);
	// 流派
	DECLARE_MEDIAMETA_TYPE(Genre, std::wstring);
};

// 媒体元数据容器
class MEDIA_CORE_API MediaMetadataContainer
{
public:
	MediaMetadataContainer()
	{

	}

	// 添加元数据
	void Add(const MediaMetadata& metadata)
	{
		metadatas.emplace(metadata.GetName(), metadata);
	}

	// 添加元数据
	void Add(MediaMetadata&& metadata)
	{
		auto name = metadata.GetName();
		metadatas.emplace(std::move(name), std::move(metadata));
	}

	// 添加元数据
	template<typename TMeta>
	void Add(typename TMeta::value_type value)
	{
		metadatas.emplace(TMeta::Name, MediaMetadata{ TMeta::Name, value });
	}

	// 获取元数据
	const any& Get(const std::wstring& name) const
	{
		return metadatas.find(name)->second.GetValue();
	}

	template<typename T>
	// 获取元数据
	T& Get(const std::wstring& name) const
	{
		return metadatas.find(name)->second.GetValue<T>();
	}

	template<typename TMeta>
	// 获取元数据
	typename TMeta::value_type& Get() const
	{
		return metadatas.find(TMeta::Name)->second.GetValue<typename TMeta::value_type>();
	}

	// 元数据是否存在
	bool Exists(const std::wstring& name) const
	{
		return metadatas.find(name) != metadatas.end();
	}

	// 获取元数据
	const any& GetOrDefault(const std::wstring& name, const any& defaultValue) const
	{
		if (Exists(name))
			return Get(name);
		return defaultValue;
	}

	template<typename T>
	// 获取元数据
	T& GetOrDefault(const std::wstring& name, T& defaultValue) const
	{
		if (Exists(name))
			return Get<T>(name);
		return defaultValue;
	}

	template<typename TMeta>
	// 获取元数据
	typename TMeta::value_type& GetOrDefault(typename TMeta::value_type& defaultValue = typename TMeta::value_type()) const
	{
		if (Exists(TMeta::Name))
			return Get<typename TMeta::value_type>(TMeta::Name);
		return defaultValue;
	}

	size_t GetSize() const noexcept
	{
		return metadatas.size();
	}
private:
	std::unordered_multimap<std::wstring, MediaMetadata> metadatas;
};

#pragma warning(pop)

NSED_TOMATO_MEDIA