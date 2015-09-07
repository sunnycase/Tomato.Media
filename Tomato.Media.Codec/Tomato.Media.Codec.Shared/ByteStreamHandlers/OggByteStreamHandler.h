//
// Tomato Media Codec
// Media Foundation Ogg ByteStream Handler
// 
// 作者：SunnyCase
// 创建时间：2015-03-17
#pragma once
#include "ByteStreamHandlerBase.h"

DEFINE_NS_MEDIA_CODEC

// Media Foundation Ogg ByteStream Handler
class DECLSPEC_UUID("B53A3B84-8492-4371-BA9D-0DBF60C69357") 
	OggByteStreamHandler : public ByteStreamHandlerBase
{
#if (NTDDI_VERSION >= NTDDI_WIN8)
	InspectableClass(L"Tomato.Media.Codec.OggByteStreamHandler", BaseTrust)
#endif
public:
	static const ByteStreamHandlerRegisterItem RegisterItems[5];

	OggByteStreamHandler();
	virtual ~OggByteStreamHandler();
protected:
	virtual void OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
		IUnknown* unkState);
private:
};

END_NS_MEDIA_CODEC