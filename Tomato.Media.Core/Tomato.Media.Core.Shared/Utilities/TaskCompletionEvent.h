//
// Tomato Media
// 任务完成事件
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../include/tomato.media.core.h"
#include <wrl.h>
#include <ppltasks.h>

namespace Tomato
{
	// 任务完成事件
	template<class TResult>
	class TaskCompletionEvent : public Microsoft::WRL::RuntimeClass<
		Microsoft::WRL::RuntimeClassFlags < Microsoft::WRL::RuntimeClassType::ClassicCom >, IUnknown>
	{
	public:
		TaskCompletionEvent()
		{

		}

		bool Set(TResult result) const
		{
			return comp_event.set(result);
		}

		template<class TException>
		bool SetException(TException exception) const
		{
			return comp_event.set_exception<TException>(exception);
		}

		concurrency::task_completion_event<TResult>& GetEvent() noexcept
		{
			return comp_event;
		}
	private:
		concurrency::task_completion_event<TResult> comp_event;
	};

	template<>
	class TaskCompletionEvent<void> : public Microsoft::WRL::RuntimeClass<
		Microsoft::WRL::RuntimeClassFlags < Microsoft::WRL::RuntimeClassType::ClassicCom >, IUnknown>
	{
	public:
		TaskCompletionEvent()
		{

		}

		bool Set() const
		{
			return comp_event.set();
		}

		template<class TException>
		bool SetException(TException exception) const
		{
			return comp_event.set_exception<TException>(exception);
		}

		concurrency::task_completion_event<void>& GetEvent() noexcept
		{
			return comp_event;
		}
	private:
		concurrency::task_completion_event<void> comp_event;
	};
}