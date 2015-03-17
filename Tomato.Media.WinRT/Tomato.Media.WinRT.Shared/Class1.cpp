#include "pch.h"
#include "Class1.h"

using namespace Tomato::Media;
using namespace Platform;
using namespace Windows::Storage;
using namespace concurrency;

std::unique_ptr<IMediaSink> sink;

String^ files[] = {
	L"04.花篝り.APE", L"03.ずるいよ….mp3"
};

Class1::Class1()
{
	sink = CreateWASAPIMediaSink();
	sink->Initialize().then([&]
	{
		create_task(Windows::ApplicationModel::Package::Current->InstalledLocation->
			GetFileAsync(files[0]))
			.then([](StorageFile^ file)
		{
			return file->OpenAsync(FileAccessMode::Read);
		}).then([&](Streams::IRandomAccessStream^ stream)
		{
			auto source = CreateRTMediaSource(stream);
			auto reader = CreateMFSourceReader(source.get());
			sink->SetMediaSourceReader(std::move(reader));
		}).then([&]
		{
			sink->StartPlayback();
		});
	});
}
