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
		_tileSet._Loaded = true;
	}
}
