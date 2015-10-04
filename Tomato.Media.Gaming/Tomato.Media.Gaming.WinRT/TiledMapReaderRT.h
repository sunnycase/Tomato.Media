//
// Tomato Media Gaming
// Tiled 地图读取器
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#pragma once
#include "common.h"
#include "TiledMapReader.h"
#include "Game.h"

DEFINE_NS_MEDIA_GAMING

public interface class ITiledMapResourceResolver
{
	Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IRandomAccessStream^>^ OnResolveTileSet(Platform::String^ name);
	Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IRandomAccessStream^>^ OnResolveImage(Platform::String^ name);
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class TiledMapReader sealed
{
public:
	TiledMapReader(Game^ game, ITiledMapResourceResolver^ resourceResolver);

	Windows::Foundation::IAsyncAction^ Parse(Windows::Storage::Streams::IRandomAccessStream^ mapStream);
private:
	std::shared_ptr<Internal::TiledMap::TiledMapReader> _mapReader;
};

END_NS_MEDIA_GAMING