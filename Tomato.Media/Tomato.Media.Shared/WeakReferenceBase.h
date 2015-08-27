//
// Tomato Media
// 弱引用基类
// 作者：SunnyCase
// 创建时间：2015-08-027
//
#pragma once
#include "common.h"
#include <memory>
#include <atomic>

DEFINE_NS_MEDIA

namespace Details
{
	template<class T>
	struct WeakReferenceContext
	{
		std::atomic<T*> ptr;

		WeakReferenceContext(T* ptr)
			:ptr(ptr)
		{

		}
	};
}

template<class T>
class WeakRef
{
public:
	WeakRef(const std::shared_ptr<Details::WeakReferenceContext<T>>& weakContext)
		:weakContext(weakContext)
	{

	}

	WRL::ComPtr<T> Resolve() const noexcept
	{
		try
		{
			if (auto context = weakContext.lock())
				return context->ptr.load(std::memory_order_consume);
		}
		catch (...) {}
		return nullptr;
	}
private:
	std::weak_ptr<Details::WeakReferenceContext<T>> weakContext;
};

template<class T, class ...TArgs>
class WeakReferenceBase : public WRL::RuntimeClass<TArgs...>
{
public:
	WeakReferenceBase()
		:weakContext(std::make_shared<Details::WeakReferenceContext<T>>(static_cast<T*>(this)))
	{

	}

	STDMETHODIMP_(ULONG) Release(void) override
	{
		ULONG ref = InternalRelease();
		if (ref == 0)
		{
			auto context = weakContext;
			if (context)
			{
				context->ptr.store(nullptr, std::memory_order_release);
				weakContext.reset();
			}
			if (GetRefCount() == 0)
			{
				delete this;

				auto module = ::Microsoft::WRL::GetModuleBase();
				if (module != nullptr)
				{
					module->DecrementObjectCount();
				}
			}
		}

		return ref;
	}

	WeakRef<T> AsWeak() const
	{
		return{ weakContext };
	}
protected:
	std::shared_ptr<Details::WeakReferenceContext<T>> weakContext;
};

END_NS_MEDIA