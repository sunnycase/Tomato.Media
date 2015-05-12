//
// Tomato Media
// Media Foundation 工作队列提供程序
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "MFWorkerQueueProvider.h"
#include "MFAsyncCallback.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

#pragma comment (lib,"Mmdevapi.lib") 
#pragma comment (lib,"Mfplat.lib") 

namespace
{
	// Media Foundation MMCSS 线程
	class MFWorkerThread : public WorkerThread
	{
	public:
		MFWorkerThread(std::function<void()>&& callback, DWORD queueId)
			:invoker(Make<MFAsyncCallback<MFWorkerThread>>(this, &MFWorkerThread::Invoke)),
			callback(std::move(callback)), queueId(queueId)
		{
			THROW_IF_FAILED(MFCreateAsyncResult(nullptr, invoker.Get(), nullptr, &invokerResult));
		}

		~MFWorkerThread()
		{
			Cancel();
		}

		virtual void Execute()
		{
			THROW_IF_FAILED(MFPutWorkItemEx2(queueId, 0, invokerResult.Get()));
		}

		virtual void Execute(const wrl::Wrappers::Event& event)
		{
			THROW_IF_FAILED(MFPutWaitingWorkItem(event.Get(), 0, invokerResult.Get(), &itemKey));
		}

		virtual void Cancel() noexcept
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
		ComPtr<MFAsyncCallback<MFWorkerThread>> invoker;
		ComPtr<IMFAsyncResult> invokerResult;
		DWORD queueId;
		std::function<void()> callback;
		MFWORKITEM_KEY itemKey = 0;
	};

	class MFSerializedWorkerQueueProvider : public WorkerQueueProvider
	{
	public:
		MFSerializedWorkerQueueProvider(DWORD parentQueueId)
		{
			THROW_IF_FAILED(MFAllocateSerialWorkQueue(parentQueueId, &queueId));
		}

		~MFSerializedWorkerQueueProvider()
		{
			MFUnlockWorkQueue(queueId);
		}

		virtual std::unique_ptr<WorkerThread> QueueWorkerThread(std::function<void()>&& callback)
		{
			return std::make_unique<MFWorkerThread>(std::move(callback), queueId);
		}
	private:
		DWORD queueId;
	};
}

MFWorkerQueueProvider::MFWorkerQueueProvider()
	:isValid(false)
{

}

MFWorkerQueueProvider::MFWorkerQueueProvider(DWORD taskId, LPCWSTR className, DWORD basePriority)
	: isValid(true)
{
	THROW_IF_FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE));
	THROW_IF_FAILED(MFLockSharedWorkQueue(className, basePriority, &taskId, &queueId));
	this->taskId = taskId;
}

MFWorkerQueueProvider::~MFWorkerQueueProvider()
{
	if (isValid)
	{
		MFUnlockWorkQueue(queueId);
		MFShutdown();
	}
}

void MFWorkerQueueProvider::Reset(DWORD taskId, LPCWSTR className, DWORD basePriority)
{
	if (isValid)
	{
		THROW_IF_FAILED(MFUnlockWorkQueue(queueId));
		isValid = false;
	}
	else
		THROW_IF_FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE));

	THROW_IF_FAILED(MFLockSharedWorkQueue(className, basePriority, &taskId, &queueId));
	this->taskId = taskId;
	isValid = true;
}

std::unique_ptr<WorkerThread> MFWorkerQueueProvider::QueueWorkerThread(std::function<void()>&& callback)
{
	THROW_IF_NOT(isValid, "Worker queue is invalid.");
	return std::make_unique<MFWorkerThread>(std::move(callback), queueId);
}

std::unique_ptr<WorkerQueueProvider> MFWorkerQueueProvider::CreateSerial()
{
	THROW_IF_NOT(isValid, "Worker queue is invalid.");
	return std::make_unique<MFSerializedWorkerQueueProvider>(queueId);
}

MFWorkerQueueProvider& MFWorkerQueueProvider::GetProAudio()
{
	static MFWorkerQueueProvider proAudioProvider(0, L"Pro Audio", 2);
	return proAudioProvider;
}

WorkerQueueProvider& WorkerQueueProvider::GetProAudio()
{
	return MFWorkerQueueProvider::GetProAudio();
}