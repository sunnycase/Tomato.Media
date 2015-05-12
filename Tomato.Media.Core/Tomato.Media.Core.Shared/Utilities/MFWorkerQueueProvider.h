//
// Tomato Media
// Media Foundation 工作队列提供程序
// 
// (c) SunnyCase 
// 创建日期 2015-05-12
#pragma once
#include "WorkerQueueProvider.h"

NSDEF_TOMATO_MEDIA

class MFWorkerQueueProvider : public WorkerQueueProvider
{
public:
	// 获取 Pro Audio 工作队列
	static MFWorkerQueueProvider& GetProAudio();

	// 无效的工作队列
	MFWorkerQueueProvider();
	MFWorkerQueueProvider(DWORD taskId, LPCWSTR className, DWORD basePriority);
	~MFWorkerQueueProvider();

	// 重新设置
	void Reset(DWORD taskId, LPCWSTR className, DWORD basePriority);
	DWORD GetTaskId() const noexcept { return taskId; }

	virtual std::unique_ptr<WorkerThread> QueueWorkerThread(std::function<void()>&& callback);
	// 创建串行工作队列
	std::unique_ptr<WorkerQueueProvider> CreateSerial();
private:
	MFWorkerQueueProvider(MFWorkerQueueProvider&) = delete;
	MFWorkerQueueProvider& operator=(MFWorkerQueueProvider&) = delete;
private:
	bool isValid;
	DWORD taskId, queueId;
};
NSED_TOMATO_MEDIA