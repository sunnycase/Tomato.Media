//
// Tomato Core
// 类型擦除
// 
// 作者：SunnyCase 
// 创建时间：2015-03-18
#pragma once
#include "Tomato.Core.h"

DEFINE_NS_CORE

class any_storage_base
{
public:
	virtual ~any_storage_base()
	{

	}

	virtual std::unique_ptr<any_storage_base> clone() const = 0;
	virtual const std::type_info& get_type() const = 0;
};

template<typename T>
class any_storage : public any_storage_base
{
public:
	any_storage(T&& value)
		:value(std::forward<T>(value))
	{

	}

	any_storage(const T& value)
		:value(value)
	{

	}

	virtual std::unique_ptr<any_storage_base> clone() const
	{
		return std::make_unique<any_storage<T>>(value);
	}

	virtual const std::type_info& get_type() const
	{
		return typeid(T);
	}

	const T& get_value() const noexcept
	{
		return value;
	}

	T& get_value() noexcept
	{
		return value;
	}
private:
	T value;
};

class any
{
public:
	template<typename T>
	using value_t = typename std::remove_reference_t<std::remove_cv_t<T>>;

	template<typename T>
	using any_storage_t = typename any_storage<value_t<T>>;

	any()
	{

	}

	any(const any& _any)
		:value(_any.empty() ? nullptr : _any.value->clone())
	{
	}

	any(any&& _any) noexcept
		:value(std::move(_any.value))
	{
	}

	template<typename T>
	explicit any(const T& value)
		: value(std::make_unique<any_storage_t<T>>(value))
	{

	}

	template<typename T>
	explicit any(T&& value)
		: value(std::make_unique<any_storage_t<T>>(std::forward<T>(value)))
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

	bool empty() const noexcept
	{
		return value == nullptr;
	}

	const std::type_info& get_runtime_type() const
	{
		return empty() ? typeid(void) : value->get_type();
	}

	const any_storage_base* get_storage() const
	{
		return value.get();
	}

	any_storage_base* get_storage()
	{
		return value.get();
	}
private:
	std::unique_ptr<any_storage_base> value;
};

END_NS_CORE

template<typename T>
NS_CORE::any::value_t<T>& any_cast(NS_CORE::any& anyValue)
{
	return static_cast<NS_CORE::any::any_storage_t<T>*>(anyValue.get_storage())->get_value();
}

template<typename T>
const NS_CORE::any::value_t<T>& any_cast(const NS_CORE::any& anyValue)
{
	return static_cast<const NS_CORE::any::any_storage_t<T>*>(anyValue.get_storage())->get_value();
}