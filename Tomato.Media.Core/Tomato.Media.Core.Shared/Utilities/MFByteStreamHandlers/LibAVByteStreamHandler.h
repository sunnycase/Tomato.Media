//
// Tomato Media
// Media Foundation LibAV ByteStream Handler
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "ByteStreamHandlerBase.h"

NSDEF_TOMATO_MEDIA

// Media Foundation LibAV ByteStream Handler
class LibAVByteStreamHandler : public ByteStreamHandlerBase
{
	InspectableClass(L"Tomato.Media.LibAVByteStreamHandler", BaseTrust)
public:
	LibAVByteStreamHandler();
	virtual ~LibAVByteStreamHandler();
protected:
	virtual void OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
		IUnknown* unkState);
private:
};

NSED_TOMATO_MEDIA