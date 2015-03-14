//
// Tomato Media
// [Internal] Media Foundation 回调
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

template<class T>
// Media Foundation 回调
class MFAsyncCallback : public wrl::RuntimeClass<wrl::RuntimeClassFlags<wrl::RuntimeClassType::ClassicCom>,
	IMFAsyncCallback>
{
public:
	typedef HRESULT(T::*Callback)(IMFAsyncResult *pAsyncResult);
	MFAsyncCallback(T* pParent, Callback callback, DWORD queueId = 0)
		:parent(pParent), callback(callback), queueId(queueId)
	{
	}

	// IMFAsyncCallback methods 
	STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
	{
		*pdwQueue = queueId;
		*pdwFlags = 0;
		return S_OK;
	}

	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult)
	{
		return (parent->*callback)(pAsyncResult);
	}

	void SetQueueId(DWORD queueId)
	{
		this->queueId = queueId;
	}
private:
	T* parent;
	Callback callback;
	DWORD queueId;
};

NSED_TOMATO_MEDIA