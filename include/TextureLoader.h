//
// Tomato Media
// 纹理加载器
// 作者：SunnyCase
// 创建时间：2015-10-03
//
#pragma once
#include "common.h"
#include "../../include/NonCopyable.h"

struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11Device;

DEFINE_NS_MEDIA

class ShaderResource
{
public:
	TOMATO_MEDIA_API ShaderResource();
	TOMATO_MEDIA_API ShaderResource(ID3D11Resource* resource, ID3D11ShaderResourceView* resourceView);
	TOMATO_MEDIA_API ShaderResource(const ShaderResource&) = default;
	TOMATO_MEDIA_API ShaderResource(ShaderResource&&) noexcept = default;
	TOMATO_MEDIA_API ~ShaderResource() = default;
	TOMATO_MEDIA_API ShaderResource& operator=(const ShaderResource&) = default;
private:
	WRL::ComPtr<ID3D11Resource> _resource;
	WRL::ComPtr<ID3D11ShaderResourceView> _resourceView;
};

// 纹理加载器
class TextureLoader : private NonCopyable
{
public:
	TOMATO_MEDIA_API TextureLoader(ID3D11Device* d3dDevice);
	TOMATO_MEDIA_API TextureLoader(TextureLoader&&) noexcept = default;
	TOMATO_MEDIA_API ~TextureLoader() = default;

	TOMATO_MEDIA_API ShaderResource CreateTexture(const byte* data, size_t length);
	TOMATO_MEDIA_API ShaderResource CreateTexture(const byte* data, size_t length, const std::wstring& mimeType);
private:
	WRL::ComPtr<ID3D11Device> _d3dDevice;
};

END_NS_MEDIA