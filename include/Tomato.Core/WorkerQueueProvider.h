//
// Tomato Media
// 工作队列提供程序
// 
// 作者：SunnyCase
// 创建日期 2015-03-15
#pragma once
#include "WorkerThread.h"

DEFINE_NS_CORE

// 工作队列提供程序
class TOMATO_CORE_API __declspec(novtable) WorkerQueueProvider
{
public:
	virtual ~WorkerQueueProvider() {}
	virtual std::shared_ptr<WorkerThread> CreateWorkerThread(std::function<void()>&& callback) = 0;
};

END_NS_CORE