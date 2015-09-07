//
// Tomato Media Codec
// Media Foundation Ogg ByteStream Handler
// 
// 作者：SunnyCase
// 创建时间：2015-03-17
#include "pch.h"
#include "OggByteStreamHandler.h"
#include "MediaSources/OggMediaSource.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#if (NTDDI_VERSION >= NTDDI_WIN8)
ActivatableClass(OggByteStreamHandler);
#else
CoCreatableClass(OggByteStreamHandler);
#endif

const ByteStreamHandlerRegisterItem OggByteStreamHandler::RegisterItems[5] =
{
	{ L".ogx", L"application/ogg" },
	{ L".ogv", L"video/ogg" },
	{ L".ogg", L"audio/ogg" },
	{ L".oga", L"audio/ogg" },
	{ L".spx", L"audio/ogg" }
};

OggByteStreamHandler::OggByteStreamHandler()
{
	
}

OggByteStreamHandler::~OggByteStreamHandler()
{

}

void OggByteStreamHandler::OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
	IUnknown* unkState)
{
	ComPtr<IMFAsyncResult> result;
	auto source = Make<OggMediaSource>();

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