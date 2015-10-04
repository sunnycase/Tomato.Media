//
// Tomato Media Gaming
// Tile 图块读取器
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#pragma once
#include "common.h"
#include "tiledmap.h"
#include <rapidjson/document.h>
#include "../../include/TextureLoader.h"
#include <bitset>
#include <d2d1.h>

DEFINE_NS_MEDIA_GAMING_INTERN
namespace TiledMap
{
	class Tile
	{
	public:
		Tile();
		void Reset();

		enum TerrainCorner : size_t
		{
			TopLeft = 0,
			TopRight = 1,
			BottomLeft = 2,
			BottomRight = 3
		};

		size_t Id;
		std::bitset<4> Terrain;
		std::unordered_map<std::wstring, std::wstring> Properties;
	};

	struct Terrain
	{
		std::wstring Name;
		size_t Tile;
	};

	// Tile 图块
	class TileSet
	{
	public:
		TileSet();
		void Reset();

		uint32_t FirstGlobalId;
		std::wstring Source;
		std::wstring Name;
		size_t TileWidth;
		size_t TileHeight;
		size_t Spacing;
		size_t Margin;
		size_t TileCount;
		std::wstring ImageSource;
		ShaderResource Image;
		std::wstring ExtraImageSource;
		ShaderResource ExtraImage;
		size_t ImageWidth;
		size_t ImageHeight;
		std::unordered_map<size_t, Tile> Tiles;
		std::vector<Terrain> Terrains;
		D2D_POINT_2L TileOffset;

		bool _Loaded;
	};

	// Tile 图块读取器
	// 调用顺序： Parse -> InitializeTileSet
	class TileSetReader : public std::enable_shared_from_this<TileSetReader>
	{
	public:
		TileSetReader(std::shared_ptr<TiledMapReaderHandler> handler);
		void Parse(const std::wstring& source);
		void Parse(const rapidjson::GenericValue<rapidjson::UTF16<>>& value);

		concurrency::task<TileSet> InitializeTileSet();
	private:
		void ParseTileSet(const rapidjson::GenericValue<rapidjson::UTF16<>>& value);
		concurrency::task<void> InitializeImage();
		void ParseTiles(const rapidjson::GenericValue<rapidjson::UTF16<>>& value);
		void ParseTerrains(const rapidjson::GenericValue<rapidjson::UTF16<>>& value);
		void ParseTileOffset(const rapidjson::GenericValue<rapidjson::UTF16<>>& value);
	private:
		std::shared_ptr<TiledMapReaderHandler> _handler;
		TileSet _tileSet;
	};
}
END_NS_MEDIA_GAMING_INTERN