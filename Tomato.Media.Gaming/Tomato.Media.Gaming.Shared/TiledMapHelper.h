//
// Tomato Media Gaming
// Tiled 地图辅助
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#pragma once
#include "common.h"
#include <ppltasks.h>
#include <unordered_map>
#include <rapidjson/document.h>

DEFINE_NS_MEDIA_GAMING_INTERN
namespace TiledMap
{
	inline std::wstring AsString(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
	{
		return{ value.GetString(), value.GetStringLength() };
	}

	void ParseProperties(const rapidjson::GenericValue<rapidjson::UTF16<>>& value, std::unordered_map<std::wstring, std::wstring>& properties);
}
END_NS_MEDIA_GAMING_INTERN