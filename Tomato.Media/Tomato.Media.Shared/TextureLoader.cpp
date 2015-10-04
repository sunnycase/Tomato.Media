//
// Tomato Media
// 纹理加载器
// 作者：SunnyCase
// 创建时间：2015-10-03
//
#include "pch.h"
#include "../../include/TextureLoader.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;
using namespace DirectX;

TextureLoader::TextureLoader(ID3D11Device * d3dDevice)
	:_d3dDevice(d3dDevice)
{
	if (!d3dDevice)
		ThrowIfFailed(E_INVALIDARG);
}

ShaderResource TextureLoader::CreateTexture(const byte * data, size_t length)
{
	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11ShaderResourceView> resourceView;
	if (FAILED(CreateDDSTextureFromMemory(_d3dDevice.Get(), data, length, &resource, &resourceView)))
		ThrowIfFailed(CreateWICTextureFromMemory(_d3dDevice.Get(), data, length, &resource, &resourceView));
	return{ resource.Get(), resourceView.Get() };
}

ShaderResource TextureLoader::CreateTexture(const byte * data, size_t length, const std::wstring & mimeType)
{
	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11ShaderResourceView> resourceView;
	auto hr = E_FAIL;
	if (mimeType == L"image/vnd-ms.dds")
		hr = CreateDDSTextureFromMemory(_d3dDevice.Get(), data, length, &resource, &resourceView);
	if (FAILED(hr))
		ThrowIfFailed(CreateWICTextureFromMemory(_d3dDevice.Get(), data, length, &resource, &resourceView));
	return{ resource.Get(), resourceView.Get() };
}

ShaderResource::ShaderResource()
{
}

ShaderResource::ShaderResource(ID3D11Resource * resource, ID3D11ShaderResourceView * resourceView)
	:_resource(resource), _resourceView(resourceView)
{
}
