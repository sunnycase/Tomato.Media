//
// Tomato Media
// 视频流渲染 Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#include "pch.h"
#include "StreamRenderSinkBase.h"
#include "MediaRenderSink.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

StreamRenderSinkBase::StreamRenderSinkBase(DWORD identifier, MediaRenderSink * mediaSink)
	:identifier(identifier), mediaSink(mediaSink)
{
	ThrowIfFailed(MFCreateEventQueue(&eventQueue));
}

StreamRenderSinkBase::~StreamRenderSinkBase()
{
	
}

HRESULT StreamRenderSinkBase::GetEvent(DWORD dwFlags, IMFMediaEvent ** ppEvent)
{
	return eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT StreamRenderSinkBase::BeginGetEvent(IMFAsyncCallback * pCallback, IUnknown * punkState)
{
	return eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT StreamRenderSinkBase::EndGetEvent(IMFAsyncResult * pResult, IMFMediaEvent ** ppEvent)
{
	return eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT StreamRenderSinkBase::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT * pvValue)
{
	return eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

HRESULT StreamRenderSinkBase::GetMediaSink(IMFMediaSink ** ppMediaSink)
{
	return mediaSink.CopyTo(ppMediaSink);
}

HRESULT StreamRenderSinkBase::GetIdentifier(DWORD * pdwIdentifier)
{
	*pdwIdentifier = identifier;
	return S_OK;
}