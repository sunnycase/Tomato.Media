//
// Tomato Media
// Media Foundation Metadata 生成器
// 作者：SunnyCase
// 创建时间：2015-12-31
//
#include "pch.h"
#include "../../include/media/MFMetadataBuilder.h"
#include "..\..\..\include\media\MFMetadataBuilder.h"

using namespace NS_MEDIA;
using namespace WRL;
using namespace concurrency;

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
		_properties.erase(it);
		return S_OK;
	}
	return MF_E_PROPERTY_NOT_FOUND;
}

HRESULT MFMetadataBuilder::GetAllPropertyNames(PROPVARIANT * ppvNames)
{
	if (_properties.empty())
		ppvNames->vt = VT_EMPTY;
	else
	{
		auto size = _properties.size();
		auto ptrs = unique_cotaskmem<LPWSTR>((LPWSTR*)CoTaskMemAlloc(size * sizeof(LPWSTR)));
		std::vector<unique_cotaskmem<WCHAR>> ptrCaches;
		if (!ptrs) return E_OUTOFMEMORY;
		size_t idx = 0;
		for (auto&& pair : _properties)
		{
			auto& key = pair.first;
			auto len = (key.size() + 1) * sizeof(wchar_t);
			auto ptr = unique_cotaskmem<WCHAR>((LPWSTR)CoTaskMemAlloc(len));
			if (!ptr) return E_OUTOFMEMORY;
			ZeroMemory(ptr.get(), len);
			if (memcpy_s(ptr.get(), len, key.data(), key.size() * sizeof(wchar_t)) != 0)
				return E_FAIL;
			ptrs.get()[idx++] = ptr.get();
			ptrCaches.emplace_back(std::move(ptr));
		}
		ppvNames->vt = VT_VECTOR | VT_LPWSTR;
		ppvNames->calpwstr.cElems = size;
		ppvNames->calpwstr.pElems = ptrs.release();
		std::for_each(ptrCaches.begin(), ptrCaches.end(), [](auto& ptr) {ptr.release();});
		return S_OK;
	}
	return S_OK;
}

void MFMetadataBuilder::SetProperty(const std::wstring & key, const std::wstring & value)
{
	auto srcLen = value.size() * sizeof(wchar_t);
	auto len = (value.size() + 1) * sizeof(wchar_t);
	auto ptr = unique_cotaskmem<WCHAR>((LPWSTR)CoTaskMemAlloc(len));
	ThrowIfNot(ptr, L"Out of memeory.");
	ZeroMemory(ptr.get(), len);
	ThrowIfNot(memcpy_s(ptr.get(), len, value.data(), srcLen) == 0, L"Cannot copy value.");

	auto it = _properties.find(key);
	if (it != _properties.end())
		PropVariantClear(&it->second);
	auto& prop = _properties[key];
	PropVariantInit(&prop);
	prop.vt = VT_LPWSTR;
	prop.pwszVal = ptr.release();
}
