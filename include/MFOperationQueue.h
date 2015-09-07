//
// Tomato Music
// Media Foundation 操作队列
// 
// 作者：SunnyCase
// 创建时间：2014-10-30
#pragma once
#include "common.h"
#include "WorkerQueueProvider.h"
#include <concurrent_queue.h>
#include <functional>

DEFINE_NS_CORE

// 操作类型
template<class TOperation>
// Media Foundation 操作队列
class MFOperationQueue final : public std::enable_shared_from_this<MFOperationQueue<TOperation>>
{
public:
	template<typename TDispatcher>
	MFOperationQueue(TDispatcher&& dispatcher)
		:dispatcher(std::forward<TDispatcher>(dispatcher))
	{

	}

	void SetWorkerQueue(WorkerQueueProvider& provider)
	{
		std::weak_ptr<MFOperationQueue> weak(shared_from_this());
		invoker = provider.CreateWorkerThread([weak]
		{
			if (auto me = weak.lock())
				me.OnProcessOperation();
		});
	}

	// 操作入队
	template<class T>
	void Enqueue(T&& operation)
	{
		if (!invoker) ThrowIfFailed(E_NOT_VALID_STATE);
		operations.push(std::forward<T>(operation));
		invoker->Execute();
	}

	// 操作入队
	template<class T>
	void EnqueueWaiting(T&& operation, const WRL::Wrappers::Event& event)
	{
		if (!invoker) ThrowIfFailed(E_NOT_VALID_STATE);
		operations.push(std::forward<T>(operation));
		invoker->Execute(event);
	}
private:
	void OnProcessOperation()
	{
		TOperation operation;
		if (operations.try_pop(operation))
			dispatcher(operation);
	}
private:
	std::shared_ptr<WorkerThread> invoker;
	std::function<void(TOperation&)> dispatcher;
	concurrency::concurrent_queue<TOperation> operations;
};

END_NS_CORE