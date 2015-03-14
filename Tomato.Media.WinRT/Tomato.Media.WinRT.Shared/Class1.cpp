#include "pch.h"
#include "Class1.h"

using namespace Tomato::Media;
using namespace Platform;

std::unique_ptr<IMediaSink> sink;

Class1::Class1()
{
	sink = IMediaSink::CreateWASAPIMediaSink();
	sink->Initialize().then([&]
	{
		sink->StartPlayback();
	});
}
