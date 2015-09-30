//
// Tomato Media Core
// Package (.pkg) 文件支持
// 作者：SunnyCase
// 创建时间：2015-09-30
//
#pragma once
#include "common.h"
#include "PackageFile.h"
#include "Tomato.Core_i.h"

DEFINE_NS_CORE

class PackageEntry : public WRL::RuntimeClass<ABI::NS_CORE::IPackageEntry>
{
	InspectableClass(RuntimeClass_Tomato_Core_PackageEntry, BaseTrust);
public:
	PackageEntry(Internal::PackageEntry&& entry);
private:
	Internal::PackageEntry _entry;
};

class PackageReader : public WRL::RuntimeClass<ABI::NS_CORE::IPackageReader>
{
	InspectableClass(RuntimeClass_Tomato_Core_PackageReader, BaseTrust);
public:
	PackageReader(HSTRING path);

	// 通过 RuntimeClass 继承
	IFACEMETHODIMP GetAllEntries(ABI::Windows::Foundation::Collections::__FIVector_1_Tomato__CCore__CPackageEntry_t *entries);
	IFACEMETHODIMP ExtractFile(ABI::Tomato::Core::IPackageEntry *entry, ABI::Windows::Storage::Streams::IBuffer *buffer);
private:
	Internal::PackageReader _reader;
};

END_NS_CORE