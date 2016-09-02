//
// Tomato
// PropVariant Helper
// 
// (c) SunnyCase 
// 创建日期 2016-09-01
#include "pch.h"
#include "Tomato.Core\PropVariant.h"

using namespace NS_CORE;

HRESULT NS_CORE::InitPropVariantFromStringVector(const WCHAR **pstr, ULONG c, PROPVARIANT *pprop)
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

		len = wcslen(pstr[i]) + 1;

		pprop->calpwstr.pElems[i] = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(len * sizeof(WCHAR)));
		if (!pprop->calpwstr.pElems[i])
		{
			PropVariantClear(pprop); /* release already allocated memory */
			return E_OUTOFMEMORY;
		}

		memcpy(pprop->calpwstr.pElems[i], pstr[i], len * sizeof(WCHAR));
	}

	return S_OK;
}

HRESULT NS_CORE::InitPropVariantFromString(PCWSTR psz, PROPVARIANT *ppropvar)
{
    ULONG len;

    len = wcslen(psz) + 1;
    ppropvar->pwszVal = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(len * sizeof(WCHAR)));
    if (ppropvar->pwszVal)
    {
        ppropvar->vt = VT_LPWSTR;
        memcpy(ppropvar->pwszVal, psz, len * sizeof(WCHAR));

        return S_OK;
    }
    else
    {
        PropVariantInit(ppropvar);
        return E_OUTOFMEMORY;
    }
}