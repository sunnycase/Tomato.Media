//
// Tomato Media
// Media Foundation Metadata 生成器
// 作者：SunnyCase
// 创建时间：2015-12-31
//
#pragma once
#include "common.h"
#include <mfidl.h>
#include <unordered_map>

DEFINE_NS_MEDIA

class MFMetadataBuilder : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFMetadata>
{
public:
	TOMATO_MEDIA_API MFMetadataBuilder();
	TOMATO_MEDIA_API virtual ~MFMetadataBuilder();

	// 通过 RuntimeClass 继承
	STDMETHODIMP SetLanguage(LPCWSTR pwszRFC1766) override;
	STDMETHODIMP GetLanguage(LPWSTR * ppwszRFC1766) override;
	STDMETHODIMP GetAllLanguages(PROPVARIANT * ppvLanguages) override;
	STDMETHODIMP SetProperty(LPCWSTR pwszName, const PROPVARIANT * ppvValue) override;
	STDMETHODIMP GetProperty(LPCWSTR pwszName, PROPVARIANT * ppvValue) override;
	STDMETHODIMP DeleteProperty(LPCWSTR pwszName) override;
	STDMETHODIMP GetAllPropertyNames(PROPVARIANT * ppvNames) override;

	TOMATO_MEDIA_API void SetProperty(const std::wstring& key, const std::wstring& value);
private:
	std::unordered_map<std::wstring, PROPVARIANT> _properties;
};

END_NS_MEDIA