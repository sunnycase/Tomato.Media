//
// Tomato
// 操作队列
// 
// 作者：SunnyCase
// 创建时间：2014-10-30
#pragma once
#include "Tomato.Core.h"
#include "WorkerQueueProvider.h"
#include <concurrent_queue.h>
#include <functional>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <vector>

DEFINE_NS_CORE

template<class TOperation, bool Waitable = false>
// 操作队列
class OperationQueue;

// 操作类型
template<class TOperation>
// Media Foundation 操作队列
class OperationQueue<TOperation, false> : public std::enable_shared_from_this<OperationQueue<TOperation>>
{
public:
	template<typename TDispatcher>
	OperationQueue(TDispatcher&& dispatcher)
		:dispatcher(std::forward<TDispatcher>(dispatcher))
	{

	}

	void SetWorkerQueue(WorkerQueueProvider& provider)
	{
		std::weak_ptr<OperationQueue> weak(shared_from_this());
		invoker = provider.CreateWorkerThread([weak]
		{
			if (auto me = weak.lock())
				me->OnProcessOperation();
		});
	}

	// 操作入队
	template<class T>
	void Enqueue(T&& operation)
	{
		if (!invoker) ThrowIfFailed(E_NOT_VALID_STATE);
		operations.push(std::forward<T>(operation));
		ActiveInvoker();
	}
private:
	void OnProcessOperation()
	{
		try
		{
			TOperation operation;
			while (operations.try_pop(operation))
				dispatcher(operation);
			invokerActive.store(false, std::memory_order_release);
		}
		catch (...)
		{
			invokerActive.store(false, std::memory_order_release);
			throw;
		}
	}
protected:
	void ActiveInvoker()
	{
		bool expect = false;
		if (invokerActive.compare_exchange_strong(expect, true))
			invoker->Execute();
	}
protected:
	std::shared_ptr<WorkerThread> invoker;
	std::function<void(TOperation&)> dispatcher;
	concurrency::concurrent_queue<TOperation> operations;
	std::atomic<bool> invokerActive = false;
};

template<class TOperation>
// Media Foundation 操作队列
class OperationQueue<TOperation, true> : public OperationQueue<TOperation, false>, public std::enable_shared_from_this<OperationQueue<TOperation, true>>
{
	struct WaitableOperation
	{
		TOperation Operation;
		HANDLE Event;

		template<class T>
		WaitableOperation(T&& operation, HANDLE event)
			:Operation(std::forward<T>(operation)), Event(event)
		{

		}
	};
public:
	using OperationQueue<TOperation, false>::OperationQueue;

	void SetWorkerQueue(WorkerQueueProvider& provider)
	{
		std::weak_ptr<OperationQueue> weak(OperationQueue<TOperation, true>::shared_from_this());
		invoker = provider.CreateWorkerThread([weak]
		{
			if (auto me = weak.lock())
				me->OnProcessOperation();
		});
	}

	using OperationQueue<TOperation, false>::Enqueue;

	// 操作入队
	template<class T>
	void Enqueue(T&& operation, const WRL::Wrappers::Event& event)
	{
		if (!invoker) ThrowIfFailed(E_NOT_VALID_STATE);
		if (!event.IsValid()) ThrowIfFailed(E_HANDLE);
		{
			std::lock_guard<decltype(waitableOperationsMutex)> locker(waitableOperationsMutex);
			waitableOperations.emplace_back(std::forward<T>(operation), event.Get());
		}
		ActiveInvoker();
	}
private:
	void OnProcessOperation()
	{
		try
		{
			while (true)
			{
				TOperation operation;
				if (operations.try_pop(operation))
					dispatcher(operation);

				bool execute = false;
				{
					std::lock_guard<decltype(waitableOperationsMutex)> locker(waitableOperationsMutex);
					if (waitableOperations.empty()) break;
					auto it = std::find_if(waitableOperations.begin(), waitableOperations.end(), [](WaitableOperation& waitable)
					{
						return WaitForSingleObject(waitable.Event, 0) == WAIT_OBJECT_0;
					});
					if (it != waitableOperations.end())
					{
						operation = std::move(it->Operation);
						execute = true;
						waitableOperations.erase(it);
					}
				}
				if (execute)
					dispatcher(operation);
			}

			invokerActive.store(false, std::memory_order_release);
		}
		catch (...)
		{
			invokerActive.store(false, std::memory_order_release);
			throw;
		}
	}
protected:
	std::vector<WaitableOperation> waitableOperations;
	std::mutex waitableOperationsMutex;
};

END_NS_CORE