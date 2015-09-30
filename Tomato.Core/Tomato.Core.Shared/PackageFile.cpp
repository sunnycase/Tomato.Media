//
// Tomato Media Core
// Package (.pkg) 文件支持
// 作者：SunnyCase
// 创建时间：2015-09-30
//
#include "pch.h"
#include "PackageFile.h"
#include <unzip.h>
#include "minizip/iowin32.h"
#include "../../include/encoding.h"

using namespace NS_CORE_INTERN;

namespace
{
	template<typename T>
	void MiniZipThrowIfNot(T value)
	{
		if (!value)
			ThrowIfFailed(E_FAIL, L"Cannot open package file.");
	}
}

PackageReader::PackageReader(const std::wstring & path)
{
	zlib_filefunc64_def filefunc;
	fill_win32_filefunc64(&filefunc);
	_file.reset(unzOpen2_64(path.c_str(), &filefunc));
	MiniZipThrowIfNot(_file.valid());
}

std::vector<PackageEntry> PackageReader::DumpEntries()
{
	std::vector<PackageEntry> entries;
	unz_file_info64 info;
	int err = unzGoToFirstFile2(_file.get(), &info, nullptr, 0, nullptr, 0, nullptr, 0);
	while (err != UNZ_END_OF_LIST_OF_FILE)
	{
		MiniZipThrowIfNot(err == UNZ_OK);
		std::string fileName(info.size_filename, '\0');
		MiniZipThrowIfNot(unzGetCurrentFileInfo64(_file.get(), nullptr, &fileName.front(), fileName.size(), nullptr, 0, nullptr, 0) == UNZ_OK);

		std::stringstream ss;
		ss << "File: " << fileName << " Compressed Size: " << info.compressed_size << " Bytes." << std::endl;
		OutputDebugStringA(ss.str().c_str());

		auto kind = fileName.back() == '\\' || fileName.back() == '/' ? PackageEntryKind::Directory : PackageEntryKind::File;
		entries.emplace_back(PackageEntry{ s2ws(fileName, CP_ACP), kind, info, unzGetOffset64(_file.get()) });
		err = unzGoToNextFile2(_file.get(), &info, nullptr, 0, nullptr, 0, nullptr, 0);
	}
	return entries;
}

void PackageReader::ExtractFile(const PackageEntry & entry, byte * buffer, size_t size)
{
	ThrowIfNot(entry.Kind == PackageEntryKind::File, L"Can only extrat file entry.");
	ThrowIfNot(size >= entry.Info.uncompressed_size, L"Not enough buffer.");
	MiniZipThrowIfNot(unzSetOffset64(_file.get(), entry.Position) == UNZ_OK);
	MiniZipThrowIfNot(unzOpenCurrentFile(_file.get()) == UNZ_OK);
	MiniZipThrowIfNot(unzReadCurrentFile(_file.get(), buffer, size));
}
