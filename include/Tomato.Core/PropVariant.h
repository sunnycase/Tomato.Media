//
// Tomato
// PropVariant Helper
// 
// (c) SunnyCase 
// 创建日期 2016-09-01
#pragma once
#include "Tomato.Core.h"

DEFINE_NS_CORE

TOMATO_CORE_API HRESULT InitPropVariantFromStringVector(const WCHAR **pstr, ULONG c, PROPVARIANT *pprop);
TOMATO_CORE_API HRESULT InitPropVariantFromString(PCWSTR psz, PROPVARIANT *ppropvar);

template<typename TIt>
HRESULT InitPropVariantFromStringVector(TIt pstr, ULONG c, PROPVARIANT *pprop)
{
	ULONG i;

	pprop->calpwstr.pElems = reinterpret_cast<LPWSTR*>(CoTaskMemAlloc(c * sizeof(WCHAR *)));
	if (!pprop->calpwstr.pElems)
		return E_OUTOFMEMORY;

	ZeroMemory(pprop->calpwstr.pElems, c * sizeof(WCHAR *));

	pprop->vt = VT_VECTOR | VT_LPWSTR;
	pprop->calpwstr.cElems = c;

	for (i = 0; i < c; ++i)
	{
		ULONG len;

		len = wcslen(*pstr) + 1;

		pprop->calpwstr.pElems[i] = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(len * sizeof(WCHAR)));
		if (!pprop->calpwstr.pElems[i])
		{
			PropVariantClear(pprop); /* release already allocated memory */
			return E_OUTOFMEMORY;
		}

		memcpy(pprop->calpwstr.pElems[i], *pstr++, len * sizeof(WCHAR));
	}

	return S_OK;
}

END_NS_CORE