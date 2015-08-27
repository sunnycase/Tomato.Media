//
// Tomato Media
// Media Foundation 工作队列提供程序
// 
// 作者：SunnyCase
// 创建日期 2015-05-12
#pragma once
#include "WorkerQueueProvider.h"
#include "NonCopyable.h"

DEFINE_NS_MEDIA

class MFWorkerQueueProviderRef : public WorkerQueueProvider
{
public:
	explicit MFWorkerQueueProviderRef(DWORD queueId) noexcept;

	///<summary>获取此工作队列是否有效</summary>
	bool IsValid() const noexcept;

	virtual std::shared_ptr<WorkerThread> CreateWorkerThread(std::function<void()> callback) override;
private:
	DWORD queueId;
};

class MFWorkerQueueProvider : public WorkerQueueProvider, NonCopyable
{
public:
	// 获取 Pro Audio 工作队列
	static MFWorkerQueueProviderRef GetAudio();
	// 获取 Pro Audio 工作队列
	static MFWorkerQueueProviderRef GetProAudio();

	MFWorkerQueueProvider(DWORD taskId, LPCWSTR className, DWORD basePriority);
	virtual ~MFWorkerQueueProvider();

	DWORD GetTaskId() const noexcept { return taskId; }

	virtual std::shared_ptr<WorkerThread> CreateWorkerThread(std::function<void()> callback) override;
	// 创建串行工作队列
	std::unique_ptr<WorkerQueueProvider> CreateSerial();
private:
	DWORD taskId, queueId;
};

END_NS_MEDIA