//
// Tomato Media
// Media Foundation Metadata 生成器
// 作者：SunnyCase
// 创建时间：2015-12-31
//
#include "pch.h"
#include <Tomato.Media/MFMetadataBuilder.h>
#include <Tomato.Core/PropVariant.h>

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace WRL;

MFMetadataBuilder::MFMetadataBuilder()
{
}

MFMetadataBuilder::~MFMetadataBuilder()
{
	for (auto&& pair : _properties)
		PropVariantClear(&pair.second);
}

HRESULT MFMetadataBuilder::SetLanguage(LPCWSTR pwszRFC1766)
{
	return E_NOTIMPL;
}

HRESULT MFMetadataBuilder::GetLanguage(LPWSTR * ppwszRFC1766)
{
	return E_NOTIMPL;
}

HRESULT MFMetadataBuilder::GetAllLanguages(PROPVARIANT * ppvLanguages)
{
	return E_NOTIMPL;
}

HRESULT MFMetadataBuilder::SetProperty(LPCWSTR pwszName, const PROPVARIANT * ppvValue)
{
	auto it = _properties.find(pwszName);
	if (it != _properties.end())
		PropVariantClear(&it->second);
	return PropVariantCopy(&_properties[pwszName], ppvValue);
}

HRESULT MFMetadataBuilder::GetProperty(LPCWSTR pwszName, PROPVARIANT * ppvValue)
{
	auto it = _properties.find(pwszName);
	if (it != _properties.end())
		return PropVariantCopy(ppvValue, &it->second);
	return MF_E_PROPERTY_NOT_FOUND;
}

HRESULT MFMetadataBuilder::DeleteProperty(LPCWSTR pwszName)
{
	auto it = _properties.find(pwszName);
	if (it != _properties.end())
	{
		PropVariantClear(&it->second);
		_properties.erase(it);
		return S_OK;
	}
	return MF_E_PROPERTY_NOT_FOUND;
}

namespace
{
	template<typename TValue>
	struct unordered_map_key_iterator
	{
		unordered_map_key_iterator(std::unordered_map<std::wstring, TValue>& map)
			:_map(map), _it(_map.begin())
		{

		}

		unordered_map_key_iterator& operator++()
		{
			_it++;
			return *this;
		}

		unordered_map_key_iterator operator++(int)
		{
			unordered_map_key_iterator other(*this);
			_it++;
			return other;
		}

		const wchar_t* operator*() const
		{
			return _it->first.c_str();
		}
	private:
		std::unordered_map<std::wstring, TValue>& _map;
		typename std::unordered_map<std::wstring, TValue>::iterator _it;
	};
}

HRESULT MFMetadataBuilder::GetAllPropertyNames(PROPVARIANT * ppvNames)
{
	try
	{
		if (_properties.empty())
		{
			ppvNames->vt = VT_EMPTY;
			return S_OK;
		}
		else
			return InitPropVariantFromStringVector(unordered_map_key_iterator<PROPVARIANT>(_properties), ULONG(_properties.size()), ppvNames);
	}
	CATCH_ALL();
}

void MFMetadataBuilder::SetProperty(const std::wstring & key, const std::wstring & value)
{
	PROPVARIANT prop;
	ThrowIfFailed(InitPropVariantFromString(value.c_str(), &prop));
	auto it = _properties.find(key);
	if (it != _properties.end())
		PropVariantClear(&it->second);
	_properties[key] = prop;
}
