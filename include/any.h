//
// Tomato Core
// 类型擦除
// 
// 作者：SunnyCase 
// 创建时间：2015-03-18
#pragma once
#include "common.h"

DEFINE_NS_CORE

class any
{
private:
	class placeholder
	{
	public:
		virtual ~placeholder()
		{

		}

		virtual std::unique_ptr<placeholder> clone() const = 0;
		virtual const std::type_info& get_type() const = 0;
	};

	template<typename T>
	class holder : public placeholder
	{
	public:
		holder(T&& value)
			:value(std::forward<T>(value))
		{

		}

		holder(const T& value)
			:value(value)
		{

		}

		virtual std::unique_ptr<placeholder> clone() const
		{
			return std::make_unique<holder<T>>(value);
		}

		virtual const std::type_info& get_type() const
		{
			return typeid(T);
		}

		const T& get_value() const noexcept
		{
			return value;
		}

		T get_value() noexcept
		{
			return value;
		}
	private:
		friend class any;
		T value;
	};
public:
	template<typename T>
	using value_t = typename std::remove_reference_t<std::remove_cv_t<T>>;

	template<typename T>
	using holder_t = typename holder<value_t<T>>;

	any()
	{

	}

	any(const any& _any)
		:value(_any.isEmpty() ? nullptr : _any.value->clone())
	{
	}

	any(any&& _any) noexcept
		:value(std::move(_any.value))
	{
	}

	template<typename T>
	explicit any(const T& value)
		: value(std::make_unique<holder_t<T>>(value))
	{

	}

	template<typename T>
	explicit any(T&& value)
		: value(std::make_unique<holder_t<T>>(std::forward<T>(value)))
	{

	}

	const any& operator= (const any& _any)
	{
		if (_any.value)
		{
			value = _any.value->clone();
		}
		return *this;
	}

	const any& operator= (any&& _any) noexcept
	{
		value = std::move(_any.value);
		return *this;
	}

	bool isEmpty() const noexcept
	{
		return value == nullptr;
	}

	const std::type_info& get_runtime_type() const
	{
		return isEmpty() ? typeid(void) : value->get_type();
	}

	const placeholder* get_placeholder() const
	{
		return value.get();
	}

	placeholder* get_placeholder()
	{
		return value.get();
	}

	static any empty;
private:
	std::unique_ptr<placeholder> value;
};

END_NS_CORE

template<typename T>
NS_CORE::any::value_t<T>& any_cast(NS_CORE::any& anyValue)
{
	return static_cast<NS_CORE::any::holder_t<T>*>(anyValue.get_placeholder())->get_value();
}

template<typename T>
const NS_CORE::any::value_t<T>& any_cast(const NS_CORE::any& anyValue)
{
	return static_cast<const NS_CORE::any::holder_t<T>*>(anyValue.get_placeholder())->get_value();
}