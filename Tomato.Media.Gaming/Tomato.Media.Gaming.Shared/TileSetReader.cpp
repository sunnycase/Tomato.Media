//
// Tomato Media Gaming
// Tile 图块读取器
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#include "pch.h"
#include "TileSetReader.h"
#include "TiledMapHelper.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_GAMING_INTERN::TiledMap;
using namespace WRL;
using namespace rapidjson;
using namespace concurrency;

TileSet::TileSet()
{
	Reset();
}

void TileSet::Reset()
{

}

TileSetReader::TileSetReader(std::shared_ptr<TiledMapReaderHandler> handler)
	:_handler(handler)
{
}

void TileSetReader::Parse(const std::wstring & source)
{
	auto document = std::make_shared<GenericDocument<UTF16<>>>();
	document->Parse(source.c_str());

	return Parse(*document);
}

void TileSetReader::Parse(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	_tileSet.Reset();
	return ParseTileSet(value);
}

concurrency::task<TileSet> TileSetReader::InitializeTileSet()
{
	if (_tileSet.Source.empty())
		return task_from_result(std::move(_tileSet));
	
	return _handler->OnReadTileSet(_tileSet.Source)
		.then([me = shared_from_this(), this](const std::wstring& source)
	{
		Parse(source);
		// 不能还是 Ref
		if (!_tileSet._Loaded)
			ThrowAlways(L"Invalid tileset file.");
		return InitializeImage();
	}).then([me = shared_from_this(), this]
	{
		return std::move(_tileSet);
	});
}

void TileSetReader::ParseTileSet(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	auto firstGid = value.FindMember(L"firstgid");
	if (firstGid != value.MemberEnd())
		_tileSet.FirstGlobalId = value[L"firstgid"].GetUint();
	auto source = value.FindMember(L"source");
	if (source != value.MemberEnd())
	{
		_tileSet.Source = AsString(value[L"source"]);
		_tileSet._Loaded = false;
	}
	else
	{
		_tileSet.Name = AsString(value[L"name"]);
		_tileSet.TileWidth = value[L"tilewidth"].GetUint();
		_tileSet.TileHeight = value[L"tileheight"].GetUint();
		_tileSet.Spacing = value[L"spacing"].GetUint();
		_tileSet.Margin = value[L"margin"].GetUint();
		_tileSet.TileCount = value[L"tilecount"].GetUint();
		_tileSet.ImageSource = AsString(value[L"image"]);
		_tileSet.ImageWidth = value[L"imagewidth"].GetUint();
		_tileSet.ImageHeight = value[L"imageheight"].GetUint();
		ParseTerrains(value);
		ParseTiles(value);
		ParseTileOffset(value);
		
		_tileSet._Loaded = true;
	}
}

void TileSetReader::ParseTiles(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	auto& tiles = _tileSet.Tiles;
	tiles.clear();
	tiles.reserve(_tileSet.TileCount);
	auto tilesMemberIt = value.FindMember(L"tiles");
	if (tilesMemberIt != value.MemberEnd())
	{
		auto& tilesMember = tilesMemberIt->value;
		ThrowIfNot(tilesMember.IsObject(), L"Invalid tiles.");
		for (auto it = tilesMember.MemberBegin(); it != tilesMember.MemberEnd();++it)
		{
			auto& tileMember = it->value;
			auto id = size_t(_wtoi(it->name.GetString()));
			auto& tile = tiles[id];
			tile.Id = id;
			ParsePropertiesMember(tileMember, tile.Properties);
			auto terrainIt = tileMember.FindMember(L"terrain");
			if (terrainIt != tileMember.MemberEnd())
			{
				auto& terrainMember = terrainIt->value;
				ThrowIfNot(terrainMember.IsArray() && terrainMember.Size() == 4, L"Invalid terrain.");
				tile.Terrain[Tile::TopLeft] = terrainMember[0].GetUint() != 0;
				tile.Terrain[Tile::TopRight] = terrainMember[1].GetUint() != 0;
				tile.Terrain[Tile::BottomLeft] = terrainMember[2].GetUint() != 0;
				tile.Terrain[Tile::BottomRight] = terrainMember[3].GetUint() != 0;
			}
		}
	}
}

void TileSetReader::ParseTerrains(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	auto& terrains = _tileSet.Terrains;
	terrains.clear();
	auto terrainsMemberIt = value.FindMember(L"terrains");
	if (terrainsMemberIt != value.MemberEnd())
	{
		auto& terrainsMember = terrainsMemberIt->value;
		ThrowIfNot(terrainsMember.IsArray(), L"Invalid terrains.");
		terrains.reserve(terrainsMember.Size());
		for (auto it = terrainsMember.Begin(); it != terrainsMember.End();++it)
		{
			auto& terrainMember = *it;
			Terrain terrain{ AsString(terrainMember[L"name"]), terrainMember[L"tile"].GetUint() };
			terrains.emplace_back(std::move(terrain));
		}
	}
}

void TileSetReader::ParseTileOffset(const rapidjson::GenericValue<rapidjson::UTF16<>>& value)
{
	_tileSet.TileOffset = {0, 0};
	auto tileOffsetIt = value.FindMember(L"tileoffset");
	if (tileOffsetIt != value.MemberEnd())
	{
		auto& tileOffsetMember = tileOffsetIt->value;
		_tileSet.TileOffset.x = tileOffsetMember[L"x"].GetInt();
		_tileSet.TileOffset.y = tileOffsetMember[L"y"].GetInt();
	}
}

concurrency::task<void> TileSetReader::InitializeImage()
{
	return _handler->OnReadImage(_tileSet.ImageSource)
		.then([me = shared_from_this(), this](const std::vector<byte>& image)
	{
		auto&& textureLoader = _handler->OnGetTextureLoader();
		_tileSet.Image = textureLoader.CreateTexture(image.data(), image.size());
	});
}

Tile::Tile()
{
}

void Tile::Reset()
{
}
