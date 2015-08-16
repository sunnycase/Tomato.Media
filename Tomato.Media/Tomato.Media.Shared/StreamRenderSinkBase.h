//
// Tomato Media
// 流渲染 Sink 基类
// 
// 作者：SunnyCase 
// 创建日期 2015-08-16
#pragma once
#include "common.h"
#include <mfidl.h>
#include <mutex>

DEFINE_NS_MEDIA

class MediaRenderSink;

///<summary>流渲染 Sink 基类</summary>
class __declspec(novtable) StreamRenderSinkBase : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFStreamSink, IMFMediaTypeHandler>
{
public:
	StreamRenderSinkBase(DWORD identifier, MediaRenderSink* mediaSink);
	virtual ~StreamRenderSinkBase();
	
	// 通过 RuntimeClass 继承
	STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent ** ppEvent) override;
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback * pCallback, IUnknown * punkState) override;
	STDMETHODIMP EndGetEvent(IMFAsyncResult * pResult, IMFMediaEvent ** ppEvent) override;
	STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT * pvValue) override;
	STDMETHODIMP GetMediaSink(IMFMediaSink ** ppMediaSink) override;
	STDMETHODIMP GetIdentifier(DWORD * pdwIdentifier) override;

	// 提示开始缓冲
	virtual void NotifyPreroll(MFTIME hnsUpcomingStartTime) = 0;
protected:
	const DWORD identifier;
	WRL::ComPtr<MediaRenderSink> mediaSink;
	WRL::ComPtr<IMFMediaEventQueue> eventQueue;
};

END_NS_MEDIA