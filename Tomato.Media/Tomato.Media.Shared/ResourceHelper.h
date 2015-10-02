//
// Tomato Media
// 资源辅助
// 作者：SunnyCase
// 创建时间：2015-08-15
//
#pragma once
#include "common.h"
#include <robuffer.h>
#include <memory>
#include <ppltasks.h>

DEFINE_NS_MEDIA

#ifdef __cplusplus_winrt

struct Resources
{
	static Platform::String^ DefaultVideoVS;
	static Platform::String^ DefaultVideoPS;
};

///<summary>二进制资源</summary>
class BlobResource
{
public:
	BlobResource(Windows::Storage::Streams::IBuffer^ buffer)
		:buffer(buffer)
	{
		auto unkBuffer = reinterpret_cast<IInspectable*>(buffer);
		ThrowIfFailed(unkBuffer->QueryInterface(IID_PPV_ARGS(&bufferAccess)));
	}

	byte* GetPointer() const
	{
		byte* pointer = nullptr;
		ThrowIfFailed(bufferAccess->Buffer(&pointer));
		return pointer;
	}

	size_t GetLength() const
	{
		return buffer->Length;
	}

	static concurrency::task<std::shared_ptr<BlobResource>> LoadFromResource(Platform::String^ name)
	{
		using namespace Windows::ApplicationModel::Resources::Core;
		using namespace Windows::Storage;
		using namespace Windows::Storage::Streams;

		auto resourceManager = ResourceManager::Current;
		auto resourceMap = resourceManager->MainResourceMap->GetSubtree(L"Files");
		auto resourceContext = ResourceContext::GetForCurrentView();
		auto candidate = resourceMap->GetValue(name, resourceContext);
		if (!candidate)
			throw Platform::Exception::CreateException(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
		return concurrency::create_task(candidate->GetValueAsFileAsync())
			.then([](StorageFile^ file)
		{
			return FileIO::ReadBufferAsync(file);
		}).then([](IBuffer^ buffer)
		{
			return std::make_shared<BlobResource>(buffer);
		});
	}
private:
	Windows::Storage::Streams::IBuffer^ buffer;
	WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> bufferAccess;
};
#else

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
#endif

END_NS_MEDIA