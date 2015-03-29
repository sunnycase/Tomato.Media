#include "pch.h"
#include "MediaSource.h"

using namespace Tomato::Media;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Media;
using namespace Windows::UI::Core;
using namespace concurrency;

inline String^ ws2RTString(const std::wstring& str)
{
	return ref new String(str.c_str());
}

MediaSource::MediaSource(Streams::IRandomAccessStream^ stream)
	:nativeSource(CreateRTMediaSource(stream))
{

}

IAsyncOperation<MediaSource^>^ MediaSource::CreateFromFile(StorageFile ^ file)
{
	return create_async([=]
	{
		return create_task(file->OpenAsync(FileAccessMode::Read))
			.then([=](Streams::IRandomAccessStream^ stream)
		{
			auto source = ref new MediaSource(stream);
			return source->Get()->Initialize().then([=]
			{
				return source;
			});
		});
	});
}

IMediaSource* MediaSource::Get()
{
	return nativeSource.get();
}

task<void> MediaSource::Initialize()
{
	return nativeSource->Initialize();
}

String^ MediaSource::Title::get()
{
	return ws2RTString(nativeSource->GetMetadatas()
		.GetOrDefault(DefaultMediaMetadatas::Title, std::wstring()));
}

String^ MediaSource::AlbumArtist::get()
{
	return ws2RTString(nativeSource->GetMetadatas()
		.GetOrDefault(DefaultMediaMetadatas::AlbumArtist, std::wstring()));
}

String^ MediaSource::Artist::get()
{
	return ws2RTString(nativeSource->GetMetadatas()
		.GetOrDefault(DefaultMediaMetadatas::Artist, std::wstring()));
}

String^ MediaSource::Album::get()
{
	return ws2RTString(nativeSource->GetMetadatas()
		.GetOrDefault(DefaultMediaMetadatas::Album, std::wstring()));
}