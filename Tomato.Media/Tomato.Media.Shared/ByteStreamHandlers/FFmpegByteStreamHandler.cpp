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

const ByteStreamHandlerRegisterInfo FFmpegByteStreamHandler::RegisterInfos[8] =
{
	{ L".ape", L"audio/x-ape" },
	{ L".ogg", L"audio/ogg" },
	{ L".m4a", L"audio/mp4a-latm" },
	{ L".aac", L"audio/aac" },
	{ L".alac", L"audio/alac" },
	{ L".dff", L"audio/dsd" },
	{ L".dsf", L"audio/dsd" },
	{ L".tak", L"audio/tak" },
};

FFmpegByteStreamHandler::FFmpegByteStreamHandler()
{

}

FFmpegByteStreamHandler::~FFmpegByteStreamHandler()
{

}

void FFmpegByteStreamHandler::OnCreateMediaSource(IMFByteStream* byteStream, LPCWSTR pwszURL, IMFAsyncCallback *callback,
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
	source->OpenAsync(byteStream).then([result](task<void> openTask)
	{
		auto hr = [=] {try
		{
			openTask.get();
			return S_OK;
		}
		CATCH_ALL();
		}();
		if (hr == E_FAIL)
			result->SetStatus(MF_E_UNSUPPORTED_BYTESTREAM_TYPE);
		else
			result->SetStatus(hr);

		MFInvokeCallback(result.Get());
	});
}