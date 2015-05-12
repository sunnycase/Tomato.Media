//
// Tomato Media
// [Internal] 工作队列提供程序
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "WorkerThread.h"

NSDEF_TOMATO_MEDIA

// 工作队列提供程序
class WorkerQueueProvider
{
public:
	// 获取 Pro Audio 工作队列
	static WorkerQueueProvider& GetProAudio();

	virtual ~WorkerQueueProvider() {}
	virtual std::unique_ptr<WorkerThread> QueueWorkerThread(std::function<void()>&& callback) = 0;
};

NSED_TOMATO_MEDIA