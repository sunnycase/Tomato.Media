//
// Tomato Media
// 媒体元数据提供程序
// 
// 作者：SunnyCase 
// 创建日期 2016-01-02
#include "pch.h"
#include "MediaMetadataProvider.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace NS_MEDIA;
using namespace NS_MEDIA_INTERN;
using namespace WRL;
using namespace concurrency;

IAsyncOperation<MediaMetadataProvider^>^ MediaMetadataProvider::CreateFromStream(IRandomAccessStream ^ stream, bool brief)
{
	auto provider = ref new MediaMetadataProvider();
	return create_async([=]
	{
		return provider->LoadAsync(stream, brief).then([=]
		{
			return provider;
		});
	});
}

MediaMetadataProvider::~MediaMetadataProvider()
{

}

MediaMetadataProvider::MediaMetadataProvider()
{

}

concurrency::task<void> MediaMetadataProvider::LoadAsync(IRandomAccessStream ^ stream, bool brief)
{
	ComPtr<IMFByteStream> byteStream;
	ThrowIfFailed(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));
	return Internal::GetMediaMetadata(byteStream.Get(), brief)
		.then([=](std::shared_ptr<MediaMetadataContainer> container)
	{
		_container = container;
	});
}

String^ MediaMetadataProvider::Title::get()
{
	auto title(_container->GetOrDefault<DefaultMediaMetadatas::Title>());
	return ref new String(title.c_str(), title.length());
}

String^ MediaMetadataProvider::AlbumArtist::get()
{
	auto albumArtist(_container->GetOrDefault<DefaultMediaMetadatas::AlbumArtist>());
	return ref new String(albumArtist.c_str(), albumArtist.length());
}

String^ MediaMetadataProvider::Artist::get()
{
	auto artist(_container->GetOrDefault<DefaultMediaMetadatas::Artist>());
	return ref new String(artist.c_str(), artist.length());
}

String^ MediaMetadataProvider::Album::get()
{
	auto album(_container->GetOrDefault<DefaultMediaMetadatas::Album>());
	return ref new String(album.c_str(), album.length());
}

Platform::IBox<TimeSpan>^ MediaMetadataProvider::Duration::get()
{
	return nullptr;
}

String^ MediaMetadataProvider::Lyrics::get()
{
	auto lyrics(_container->GetOrDefault<DefaultMediaMetadatas::Lyrics>());
	return ref new String(lyrics.c_str(), lyrics.length());
}

IVector<NS_MEDIA::Picture^>^ MediaMetadataProvider::Pictures::get()
{
	std::vector<NS_MEDIA::Picture^> pictures;
	_container->ForEach<DefaultMediaMetadatas::Picture>([&](const Internal::Picture& picture)
	{
		pictures.emplace_back(ref new NS_MEDIA::Picture(picture));
		return false;
	});
	return ref new Platform::Collections::Vector<NS_MEDIA::Picture^>(std::move(pictures));
}

NS_MEDIA::Picture::Picture(const Internal::Picture& picture)
	:_mimeType(ref new String(picture.MimeType.c_str(), picture.MimeType.length())),
	_description(ref new String(picture.Description.c_str(), picture.Description.length())),
	_pictureType(ref new String(picture.Type.c_str(), picture.Type.length())),
	_data(ref new Platform::Array<byte>(const_cast<byte*>(picture.Data.data()), picture.Data.size()))
{

}