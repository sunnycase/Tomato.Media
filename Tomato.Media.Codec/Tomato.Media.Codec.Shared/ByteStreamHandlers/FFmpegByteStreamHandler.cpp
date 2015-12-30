//
// Tomato Media Codec
// Media Foundation FFmpeg ByteStream Handler
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#include "pch.h"
#include "FFmpegByteStreamHandler.h"
#include "MediaSources/FFmpegMediaSource.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#if (NTDDI_VERSION >= NTDDI_WIN8)
ActivatableClass(FFmpegByteStreamHandler);
#else
CoCreatableClass(FFmpegByteStreamHandler);
#endif

const ByteStreamHandlerRegisterInfo FFmpegByteStreamHandler::RegisterInfos[1] =
{
	{ L".ape", L"audio/x-ape" }
};

FFmpegByteStreamHandler::FFmpegByteStreamHandler()
{
	
}

FFmpegByteStreamHandler::~FFmpegByteStreamHandler()
{

}

void FFmpegByteStreamHandler::OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
	IUnknown* unkState)
{
	ComPtr<IMFAsyncResult> result;
	auto source = Make<FFmpegMediaSource>();

	ComPtr<IUnknown> unkSource;
	ThrowIfFailed(source.As(&unkSource));
	ThrowIfFailed(MFCreateAsyncResult(unkSource.Get(), callback, unkState, &result));

	// Start opening the source. This is an async operation.
	// When it completes, the source will invoke our callback
	// and then we will invoke the caller's callback.
	source->OpenAsync(byteStream).then([](task<void> openTask) -> HRESULT
	{
		try
		{
			openTask.get();
			return S_OK;
		}
		CATCH_ALL();
	}).then([result](HRESULT hr)
	{
		if (hr == E_FAIL)
			result->SetStatus(MF_E_UNSUPPORTED_BYTESTREAM_TYPE);
		else
			result->SetStatus(hr);

		MFInvokeCallback(result.Get());
	});
}