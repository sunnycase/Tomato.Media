//
// Tomato Media
// Media Foundation LibAV ByteStream Handler
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#include "pch.h"
#include "LibAVByteStreamHandler.h"
#include "../MFSources/LibAVAudioSource.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

ActivatableClass(LibAVByteStreamHandler);

LibAVByteStreamHandler::LibAVByteStreamHandler()
{

}

LibAVByteStreamHandler::~LibAVByteStreamHandler()
{

}

void LibAVByteStreamHandler::OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
	IUnknown* unkState)
{
	ComPtr<IMFAsyncResult> result;
	auto source = Make<LibAVAudioSource>();

	ComPtr<IUnknown> unkSource;
	THROW_IF_FAILED(source.As(&unkSource));
	THROW_IF_FAILED(MFCreateAsyncResult(unkSource.Get(), callback, unkState, &result));

	// Start opening the source. This is an async operation.
	// When it completes, the source will invoke our callback
	// and then we will invoke the caller's callback.
	source->OpenAsync(byteStream).then([result, source](task<void> openTask)
	{
		if (result == nullptr)
			return;
		try
		{
			openTask.get();
		}
		catch (Platform::Exception ^ex)
		{
			result->SetStatus(ex->HResult);
		}
		catch (...)
		{
			result->SetStatus(MF_E_UNSUPPORTED_BYTESTREAM_TYPE);
		}

		MFInvokeCallback(result.Get());
	});
}