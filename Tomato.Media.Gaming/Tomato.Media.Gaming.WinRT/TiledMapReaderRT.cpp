//
// Tomato Media Gaming
// Tiled 地图读取器
// 
// 作者：SunnyCase 
// 创建日期 2015-10-03
#include "pch.h"
#include "TiledMapReaderRT.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_GAMING;
using namespace WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace concurrency;

namespace
{
	task<std::wstring> ReadAllText(IRandomAccessStream^ stream)
	{
		auto buffer = ref new Buffer(stream->Size);
		return create_task(stream->ReadAsync(buffer, stream->Size, InputStreamOptions::None))
			.then([](IBuffer^ buffer)
		{
			auto dataReader = DataReader::FromBuffer(buffer);
			auto str = dataReader->ReadString(buffer->Length);
			return std::wstring(str->Begin(), str->End());
		});
	}

	class TiledMapReaderHandler : public Internal::TiledMap::TiledMapReaderHandler
	{
	public:
		TiledMapReaderHandler(ITiledMapResourceResolver^ resourceResolver)
			:_resourceResolver(resourceResolver)
		{

		}

		// 通过 TiledMapReader 继承
		virtual concurrency::task<std::wstring> OnReadTileSet(const std::wstring & name) const override
		{
			return create_task(_resourceResolver->OnResolveTileSet(ref new String(name.c_str(), name.length())))
				.then([](IRandomAccessStream^ stream)
			{
				return ReadAllText(stream);
			});
		}
	private:
		ITiledMapResourceResolver^ _resourceResolver;
	};
}

TiledMapReader::TiledMapReader(ITiledMapResourceResolver^ resourceResolver)
	:_mapReader(std::make_shared<Internal::TiledMap::TiledMapReader>(
		std::make_shared<TiledMapReaderHandler>(resourceResolver)))
{

}

IAsyncAction ^ TiledMapReader::Parse(IRandomAccessStream ^ mapStream)
{
	return create_async([=]
	{
		return ReadAllText(mapStream).then([this](const std::wstring& source)
		{
			return _mapReader->Read(source)
				.then([](Internal::TiledMap::TiledMap& map)
			{

			});
		});
	});
}
