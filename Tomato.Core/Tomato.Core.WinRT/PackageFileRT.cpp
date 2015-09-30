//
// Tomato Media Core
// Package (.pkg) 文件支持
// 作者：SunnyCase
// 创建时间：2015-09-30
//
#include "pch.h"
#include "PackageFileRT.h"

using namespace NS_CORE;
using namespace WRL;
using namespace WRL::Wrappers;
using namespace ABI::Windows::Foundation::Collections;

PackageReader::PackageReader(HSTRING path)
	:_reader(HStringToWString(path))
{

}

HRESULT PackageReader::GetAllEntries(__FIVector_1_Tomato__CCore__CPackageEntry_t *entries)
{
	try
	{
		for (auto&& entry : _reader.DumpEntries())
		{
			auto myEntry = Make<PackageEntry>(std::move(entry));
			ThrowIfFailed(entries->Append(myEntry.Get()));
		}
		return S_OK;
	}
	CATCH_ALL();
}

HRESULT PackageReader::ExtractFile(ABI::Tomato::Core::IPackageEntry *entry, ABI::Windows::Storage::Streams::IBuffer *buffer)
{
	try
	{
		ComPtr<Windows::Storage::Streams::IBufferByteAccess> bufferAccess;
		ThrowIfFailed(buffer->QueryInterface(IID_PPV_ARGS(&bufferAccess)));


		return S_OK;
	}
	CATCH_ALL();
}

PackageEntry::PackageEntry(Internal::PackageEntry && entry)
	:_entry(std::move(entry))
{
}
