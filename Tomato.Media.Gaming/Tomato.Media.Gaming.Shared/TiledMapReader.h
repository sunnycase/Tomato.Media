//
// Tomato Media Gaming
// Tiled 地图读取器
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#pragma once
#include "common.h"
#include "tiledmap.h"
#include "TileSetReader.h"

DEFINE_NS_MEDIA_GAMING_INTERN
namespace TiledMap
{
	enum class Orientation
	{
		Orthogonal,
		Isometric,
		Hexagonal,
		Shifted
	};

	enum class RenderOrder
	{
		RightUp,
		RightDown,
		LeftDown,
		LeftUp
	};

	struct Layer
	{
		std::wstring Name;
		double Opacity;
		bool Visible;
	};

	class TiledMap
	{
	public:
		TiledMap();
		void Reset();

		double Version;
		Orientation Orientation;
		RenderOrder RenderOrder;
		size_t Width;
		size_t Height;
		size_t TileWidth;
		size_t TileHeight;
		std::unordered_map<std::wstring, std::wstring> Properties;
		std::vector<TileSet> Tilesets;
		std::vector<Layer> Layers;
	};

	// Tiled 地图读取器
	// 调用顺序： Read
	class TiledMapReader : public std::enable_shared_from_this<TiledMapReader>
	{
	public:
		TiledMapReader(std::shared_ptr<TiledMapReaderHandler> handler);
		concurrency::task<TiledMap> Read(const std::wstring& source);
	private:
		concurrency::task<void> InitializeTiledMap();
		void ParseMap(const std::wstring& source);
		void ParseTilesets(const rapidjson::GenericValue<rapidjson::UTF16<>>& tilesets);
		void ParseLayers(const rapidjson::GenericValue<rapidjson::UTF16<>>& value);
	private:
		std::vector<std::shared_ptr<TileSetReader>> _tileSetReaders;
		std::shared_ptr<TiledMapReaderHandler> _handler;
		TiledMap _tileMap;
	};
}

END_NS_MEDIA_GAMING_INTERN