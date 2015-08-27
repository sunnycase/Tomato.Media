//
// Tomato Media
// Media Foundation 工作队列提供程序
// 
// 作者：SunnyCase
// 创建日期 2015-03-15
#include "pch.h"
#include "MFWorkerQueueProvider.h"
#include "MFAsyncCallback.h"

using namespace NS_MEDIA;
using namespace WRL;
using namespace concurrency;

namespace
{
	// Media Foundation MMCSS 线程
	class MFWorkerThread : public WorkerThread, public std::enable_shared_from_this<MFWorkerThread>
	{
	public:
		MFWorkerThread(std::function<void()> callback, DWORD queueId)
			:queueId(queueId)
		{
			assert(callback);
			auto asyncCallback = Make<MFAsyncCallback<MFWorkerThread>>(shared_from_this(),
				&MFWorkerThread::Invoke, queueId);
			ThrowIfFailed(MFCreateAsyncResult(nullptr, asyncCallback.Get(), nullptr, &invokerResult));
		}

		~MFWorkerThread()
		{
			Cancel();
		}

		virtual void Execute() override
		{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			ThrowIfFailed(MFPutWorkItemEx2(queueId, 0, invokerResult.Get()));
#endif
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			ThrowIfFailed(MFPutWorkItemEx(queueId, invokerResult.Get()));
#endif
		}

		virtual void Execute(const Wrappers::Event& event) override
		{
#if (WINVER >= _WIN32_WINNT_WIN7)
			ThrowIfFailed(MFPutWaitingWorkItem(event.Get(), 0, invokerResult.Get(), &itemKey));
#endif
		}

		virtual void Cancel() noexcept override
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
				callback();
			}
			CATCH_ALL();
			return S_OK;
		}
	private:
		ComPtr<IMFAsyncResult> invokerResult;
		const DWORD queueId;
		std::function<void()> callback;
		MFWORKITEM_KEY itemKey = 0;
	};

	class MFSerializedWorkerQueueProvider : public WorkerQueueProvider
	{
	public:
		MFSerializedWorkerQueueProvider(DWORD parentQueueId)
		{
			ThrowIfFailed(MFAllocateSerialWorkQueue(parentQueueId, &queueId));
		}

		~MFSerializedWorkerQueueProvider()
		{
			MFUnlockWorkQueue(queueId);
		}

		virtual std::shared_ptr<WorkerThread> CreateWorkerThread(std::function<void()> callback) override
		{
			return std::make_shared<MFWorkerThread>(std::move(callback), queueId);
		}
	private:
		DWORD queueId;
	};
}

MFWorkerQueueProvider::MFWorkerQueueProvider(DWORD taskId, LPCWSTR className, DWORD basePriority)
{
	ThrowIfFailed(MFLockSharedWorkQueue(className, basePriority, &taskId, &queueId));
	this->taskId = taskId;
}

MFWorkerQueueProvider::~MFWorkerQueueProvider()
{
	MFUnlockWorkQueue(queueId);
}

std::shared_ptr<WorkerThread> MFWorkerQueueProvider::CreateWorkerThread(std::function<void()> callback)
{
	return std::make_shared<MFWorkerThread>(std::move(callback), queueId);
}

std::unique_ptr<WorkerQueueProvider> MFWorkerQueueProvider::CreateSerial()
{
	return std::make_unique<MFSerializedWorkerQueueProvider>(queueId);
}

MFWorkerQueueProviderRef MFWorkerQueueProvider::GetAudio()
{
	static MFWorkerQueueProvider audioProvider(0, L"Audio", 0);
	return MFWorkerQueueProviderRef(audioProvider.queueId);
}

MFWorkerQueueProviderRef MFWorkerQueueProvider::GetProAudio()
{
	static MFWorkerQueueProvider proAudioProvider(0, L"Pro Audio", 0);
	return MFWorkerQueueProviderRef(proAudioProvider.queueId);
}

MFWorkerQueueProviderRef::MFWorkerQueueProviderRef(DWORD queueId) noexcept
	:queueId(queueId)
{
}

bool MFWorkerQueueProviderRef::IsValid() const noexcept
{
	return queueId != MFASYNC_CALLBACK_QUEUE_UNDEFINED;
}

std::shared_ptr<WorkerThread> MFWorkerQueueProviderRef::CreateWorkerThread(std::function<void()> callback)
{
	if (!IsValid())
		ThrowIfFailed(E_NOT_VALID_STATE, L"无效的工作队列。");

	return std::make_shared<MFWorkerThread>(std::move(callback), queueId);
}
