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
	T& GetValue() const { return value.get<T>() }

	// 设置值
	void SetValue(const any& value) { this->value = value; }
	// 设置值
	void SetValue(any&& value) { this->value = std::move(value); }
private:
	std::wstring name;
	any value;
};

class DefaultMediaMetadatas
{
public:
	// 标题
	MEDIA_CORE_API static const std::wstring Title;
	// 专辑
	MEDIA_CORE_API static const std::wstring Album;
	// 专辑艺术家
	MEDIA_CORE_API static const std::wstring AlbumArtist;
	// 艺术家
	MEDIA_CORE_API static const std::wstring Artist;
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
private:
	std::unordered_map<std::wstring, MediaMetadata> metadatas;
};

NSED_TOMATO_MEDIA