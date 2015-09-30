//
// Tomato Media Core
// Package (.pkg) 文件支持
// 作者：SunnyCase
// 创建时间：2015-09-30
//
#pragma once
#include "common.h"
#include <string>
#include <ppltasks.h>
#include <minizip_raii.h>

DEFINE_NS_CORE_INTERN

enum class PackageEntryKind
{
	Directory,
	File
};

struct PackageEntry
{
	std::wstring Path;
	PackageEntryKind Kind;
	unz_file_info64 Info;
	ZPOS64_T Position;
};

// Package (.pkg) 文件读取器
class PackageReader
{
public:
	PackageReader(const std::wstring& path);

	std::vector<PackageEntry> DumpEntries();
	void ExtractFile(const PackageEntry& entry, byte* buffer, size_t size);
private:
	unzFile_raii _file;
};

END_NS_CORE_INTERN