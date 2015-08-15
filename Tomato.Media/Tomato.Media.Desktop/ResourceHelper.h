//
// Tomato Media
// 资源辅助
// 作者：SunnyCase
// 创建时间：2015-08-15
//
#pragma once
#include "common.h"
#include <memory>
#include <ppltasks.h>

DEFINE_NS_MEDIA

struct ResourceRef
{
	LPCTSTR Name;
	LPCTSTR Type;
};

struct Resources
{
	static const ResourceRef DefaultVideoVS;
	static const ResourceRef DefaultVideoPS;
};

///<summary>二进制资源</summary>
class BlobResource
{
public:
	BlobResource(HRSRC resInfo, HGLOBAL handle)
		:resInfo(resInfo), handle(handle)
	{

	}

	byte* GetPointer() const
	{
		byte* pointer;
		ThrowWin32IfNot(pointer = reinterpret_cast<byte*>(LockResource(handle)));
		return pointer;
	}

	size_t GetLength() const
	{
		size_t size;
		ThrowWin32IfNot(size = SizeofResource(ModuleHandle, resInfo));
		return size;
	}

	static concurrency::task<std::shared_ptr<BlobResource>> LoadFromResource(const ResourceRef& res)
	{
		auto resInfo = FindResource(ModuleHandle, res.Name, res.Type);
		ThrowWin32IfNot(resInfo);
		auto handle = LoadResource(ModuleHandle, resInfo);
		ThrowWin32IfNot(handle);
		return concurrency::task_from_result(std::make_shared<BlobResource>(resInfo, handle));
	}
private:
	HRSRC resInfo;
	HGLOBAL handle;
};

END_NS_MEDIA