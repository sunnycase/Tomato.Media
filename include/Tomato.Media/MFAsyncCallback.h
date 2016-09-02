//
// Tomato Media
// Media Foundation 回调
// 
// 作者：SunnyCase 
// 创建日期 2015-03-15
// 修改记录：
// 2015-08-05 支持 TPL
#pragma once
#include "Tomato.Media.h"
#include <mfidl.h>
#include <ppltasks.h>
#include <memory>
#include <Tomato.Core/WeakReferenceBase.h>

DEFINE_NS_MEDIA

template<class T>
// Media Foundation 回调
class MFAsyncCallback : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
	IMFAsyncCallback>
{
public:
	typedef HRESULT(T::*Callback)(IMFAsyncResult *pAsyncResult);
	MFAsyncCallback(const std::shared_ptr<T>& pParent, Callback callback, DWORD queueId = 0)
		:parent(pParent), callback(callback), queueId(queueId)
	{
	}

	// IMFAsyncCallback methods 
	STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue) override
	{
		*pdwQueue = queueId;
		*pdwFlags = 0;
		return S_OK;
	}

	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override
	{
		if (auto validPtr = parent.lock())
		{
			try
			{
				return (validPtr.get()->*callback)(pAsyncResult);
			}
			CATCH_ALL();
		}
		return MF_E_SHUTDOWN;
	}

	DWORD GetQueueId() const noexcept
	{
		return queueId;
	}
private:
	std::weak_ptr<T> parent;
	Callback callback;
	const DWORD queueId;
};

template<class T, class Base = T>
// Media Foundation 回调
class MFAsyncCallbackWithWeakRef : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
	IMFAsyncCallback>
{
public:
	typedef void(T::*Callback)(IMFAsyncResult *pAsyncResult);
	MFAsyncCallbackWithWeakRef(Core::WeakRef<Base> pParent, Callback callback, DWORD queueId = 0)
		:parent(std::move(pParent)), callback(callback), queueId(queueId)
	{
	}

	// IMFAsyncCallback methods 
	STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue) override
	{
		*pdwQueue = queueId;
		*pdwFlags = 0;
		return S_OK;
	}

	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override
	{
		if (auto validPtr = parent.Resolve<T>())
		{
			try
			{
				(validPtr.Get()->*callback)(pAsyncResult);
				return S_OK;
			}
			CATCH_ALL();
		}
		return MF_E_SHUTDOWN;
	}

	DWORD GetQueueId() const noexcept
	{
		return queueId;
	}
private:
	Core::WeakRef<Base> parent;
	Callback callback;
	const DWORD queueId;
};

template<class T>
// Media Foundation 回调
class MFAsyncTaskCallback : public MFAsyncCallback<T>
{
public:
	typedef HRESULT(T::*Callback)(IMFAsyncResult *pAsyncResult);
	MFAsyncTaskCallback(const std::shared_ptr<T>& pParent, Callback callback, DWORD queueId = 0)
		:MFAsyncCallback<T>(pParent, callback, queueId)
	{
	}

	MFAsyncTaskCallback(DWORD queueId = 0)
		:MFAsyncCallback<T>(queueId)
	{
	}

	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult)
	{
		auto hr = MFAsyncCallback<T>::Invoke(pAsyncResult);
		if (SUCCEEDED(hr))
			completionEvent.set();
		else
			completionEvent.set_exception(hr);
		return hr;
	}

	const concurrency::task_completion_event<void>& GetEvent() const noexcept
	{
		return completionEvent;
	}
private:
	concurrency::task_completion_event<void> completionEvent;
};

END_NS_MEDIA