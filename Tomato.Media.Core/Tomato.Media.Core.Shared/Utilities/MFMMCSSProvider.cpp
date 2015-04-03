//
// Tomato Media
// Media Foundation MMCSS 提供程序
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "MFMMCSSProvider.h"
#include "MFAsyncCallback.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

#pragma comment (lib,"Mmdevapi.lib") 
#pragma comment (lib,"Mfplat.lib") 

// Media Foundation MMCSS 线程
class MFMMCSSThread : public MMCSSThread
{
public:
	MFMMCSSThread(std::function<void()>&& callback, DWORD queueId)
		:invoker(Make<MFAsyncCallback<MFMMCSSThread>>(this, &MFMMCSSThread::Invoke)),
		callback(std::move(callback)), queueId(queueId)
	{
		THROW_IF_FAILED(MFCreateAsyncResult(nullptr, invoker.Get(), nullptr, &invokerResult));
	}

	virtual void Execute()
	{
		THROW_IF_FAILED(MFPutWorkItemEx2(queueId, 0, invokerResult.Get()));
	}

	virtual void Execute(const wrl::Wrappers::Event& event)
	{
		THROW_IF_FAILED(MFPutWaitingWorkItem(event.Get(), 0, invokerResult.Get(), &itemKey));
	}

	virtual void Cancel()
	{
		if (itemKey)
		{
			MFCancelWorkItem(itemKey);
			itemKey = 0;
		}
	}
private:
	HRESULT Invoke(IMFAsyncResult *pAsyncResult) noexcept
	{
		try
		{
			if (callback)
				callback();
		}
		CATCH_ALL();
		return S_OK;
	}
private:
	ComPtr<MFAsyncCallback<MFMMCSSThread>> invoker;
	ComPtr<IMFAsyncResult> invokerResult;
	DWORD queueId;
	std::function<void()> callback;
	MFWORKITEM_KEY itemKey = 0;
};

MFMMCSSProvider::MFMMCSSProvider()
{
	THROW_IF_FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE));
	THROW_IF_FAILED(MFLockSharedWorkQueue(L"Pro Audio", 0, &taskId, &queueId));
}

MFMMCSSProvider::~MFMMCSSProvider() noexcept
{
	MFUnlockWorkQueue(queueId);
	MFShutdown();
}

std::unique_ptr<MMCSSThread> MFMMCSSProvider::CreateMMCSSThread(std::function<void()>&& callback)
{
	return std::make_unique<MFMMCSSThread>(std::move(callback), queueId);
}

MFMMCSSProvider& MFMMCSSProvider::GetDefault()
{
	static MFMMCSSProvider defaultProvider;
	return defaultProvider;
}