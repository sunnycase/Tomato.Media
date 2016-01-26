//
// Tomato Media Codec
// Media Foundation FFmpeg ByteStream Handler
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#pragma once
#include "ByteStreamHandlerBase.h"
#if (NTDDI_VERSION >= NTDDI_WIN8)
#include "Tomato.Media.Codec_i.h"
#endif

DEFINE_NS_MEDIA_CODEC

// Media Foundation FFmpeg ByteStream Handler
class DECLSPEC_UUID("DAD2BA36-EB36-473B-B64C-5B7395FCE8D0")
	FFmpegByteStreamHandler : public ByteStreamHandlerBase
{
#if (NTDDI_VERSION >= NTDDI_WIN8)
	InspectableClass(RuntimeClass_Tomato_Media_Codec_FFmpegByteStreamHandler, BaseTrust)
#endif
public:
	static const ByteStreamHandlerRegisterInfo RegisterInfos[9];

	FFmpegByteStreamHandler();
	virtual ~FFmpegByteStreamHandler();
protected:
	virtual void OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
		IUnknown* unkState);
private:
};

END_NS_MEDIA_CODEC