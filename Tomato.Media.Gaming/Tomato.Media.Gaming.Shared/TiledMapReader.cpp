//
// Tomato Media Gaming
// Tiled 地图读取器
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#include "pch.h"
#include "TiledMapReader.h"
#include "TiledMapHelper.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_GAMING_INTERN::TiledMap;
using namespace WRL;
using namespace rapidjson;
using namespace concurrency;

namespace
{
	Orientation ParseOrientation(const GenericValue<UTF16<>>& value)
	{
		const auto begin = value.GetString();
		const auto end = begin + value.GetStringLength();
		if (std::equal(begin, end, L"orthogonal"))
			return Orientation::Orthogonal;
		else if (std::equal(begin, end, L"isometric"))
			return Orientation::Isometric;
		else if (std::equal(begin, end, L"hexagonal"))
			return Orientation::Hexagonal;
		else if (std::equal(begin, end, L"shifted"))
			return Orientation::Shifted;
		ThrowAlways(L"Invalid Orientation.");
	}

	RenderOrder ParseRenderOrder(const GenericValue<UTF16<>>& value)
	{
		const auto begin = value.GetString();
		const auto end = begin + value.GetStringLength();
		if (std::equal(begin, end, L"right-down"))
			return RenderOrder::RightDown;
		else if(std::equal(begin, end, L"right-up"))
			return RenderOrder::RightUp;
		else if (std::equal(begin, end, L"left-down"))
			return RenderOrder::LeftDown;
		else if (std::equal(begin, end, L"left-up"))
			return RenderOrder::LeftUp;
		ThrowAlways(L"Invalid Render Order.");
	}
}

TiledMapReader::TiledMapReader(std::shared_ptr<TiledMapReaderHandler> handler)
	:_handler(handler)
{
}

concurrency::task<TiledMap> TiledMapReader::Read(const std::wstring & source)
{
	_tileMap.Reset();
	ParseMap(source);
	return InitializeTiledMap().then([me = shared_from_this()]
	{
		return std::move(me->_tileMap);
	});
}

concurrency::task<void> TiledMapReader::InitializeTiledMap()
{
	std::vector<task<TileSet>> tilesetsInitTasks;
	tilesetsInitTasks.reserve(_tileSetReaders.size());
	for (auto&& tilesetReader : _tileSetReaders)
		tilesetsInitTasks.emplace_back(tilesetReader->InitializeTileSet());

	return when_all(tilesetsInitTasks.begin(), tilesetsInitTasks.end())
		.then([me = shared_from_this(), this](std::vector<TileSet>& tilesets)
	{
		_tileMap.Tilesets = std::move(tilesets);
	});
}

void TiledMapReader::ParseMap(const std::wstring & source)
{
	GenericDocument<UTF16<>> document;
	
	document.Parse(source.c_str());
	_tileMap.Version = document[L"version"].GetDouble();
	_tileMap.Orientation = ParseOrientation(document[L"orientation"]);
	_tileMap.RenderOrder = ParseRenderOrder(document[L"renderorder"]);
	_tileMap.Width = document[L"width"].GetUint();
	_tileMap.Height = document[L"height"].GetUint();
	_tileMap.TileWidth = document[L"tilewidth"].GetUint();
	_tileMap.TileHeight = document[L"tileheight"].GetUint();
	ParsePropertiesMember(document, _tileMap.Properties);
	ParseTilesets(document);
}

void TiledMapReader::ParseTilesets(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	auto tilesetsIt = value.FindMember(L"tilesets");
	if (tilesetsIt != value.MemberEnd())
	{
		auto& tilesets = tilesetsIt->value;
		ThrowIfNot(tilesets.IsArray(), L"Invalid tilesets.");
		_tileSetReaders.clear();
		_tileSetReaders.reserve(tilesets.Size());
		for (auto it = tilesets.Begin(); it != tilesets.End();++it)
		{
			auto tilesetReader = std::make_shared<TileSetReader>(_handler);
			tilesetReader->Parse(*it);
			_tileSetReaders.emplace_back(std::move(tilesetReader));
		}
	}
}

void TiledMapReader::ParseLayers(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	auto layersIt = value.FindMember(L"layers");
	if (layersIt != value.MemberEnd())
	{
		auto& layersMember = layersIt->value;
		ThrowIfNot(layersMember.IsArray(), L"Invalid layers.");
		auto& layers = _tileMap.Layers;
		layers.reserve(layersMember.MemberCount());
		for (auto it = layersMember.Begin(); it != layersMember.End(); ++it)
		{
			auto& layerMember = *it;
			ThrowIfNot(layerMember.IsObject(), L"Invalid layer.");
			layers.emplace_back();
			auto& layer = layers.back();
			layer.Name = AsString(layerMember[L"name"]);
			layer.Opacity = layerMember[L"opacity"].GetDouble();
			layer.Visible = layerMember[L"visible"].GetBool();
		}
	}
}

TiledMap::TiledMap()
{
	Reset();
}

void TiledMap::Reset()
{
	Properties.clear();
}
