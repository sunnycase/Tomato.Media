//
// Tomato Music
// Media Foundation 操作队列
// 
// (c) SunnyCase 
// 创建日期 2014-10-30
#pragma once
#include "WorkerQueueProvider.h"
#include <concurrent_queue.h>

namespace Tomato
{
	// 操作类型
	template<class TOperation>
	// Media Foundation 操作队列
	class MFOperationQueue
	{
	public:
		using operation_t = TOperation;
	protected:
		MFOperationQueue(Media::WorkerQueueProvider& mmcss)
			:invoker(mmcss.QueueWorkerThread(std::bind(
			&MFOperationQueue::OnProcessOperation, this)))
		{

		}
	public:
		// 操作入队
		void Enqueue(operation_t&& operation)
		{
			operations.push(std::move(operation));
			invoker->Execute();
		}

		// 操作入队
		void EnqueueWaiting(operation_t&& operation, const wrl::Wrappers::Event& event)
		{
			operations.push(std::move(operation));
			invoker->Execute(event);
		}
	protected:
		// 发送操作
		virtual void DispatchOperation(TOperation& op) = 0;
		// 验证操作
		virtual void ValidateOperation(TOperation& op) = 0;
	private:
		void OnProcessOperation()
		{
			TOperation operation;
			if (operations.try_pop(operation))
			{
				ValidateOperation(operation);
				DispatchOperation(operation);
			}
		}
	private:
		std::unique_ptr<Media::WorkerThread> invoker;
		concurrency::concurrent_queue<TOperation> operations;
	};
}