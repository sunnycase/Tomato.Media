//
// Tomato Media
// WIC 辅助
// 作者：SunnyCase
// 创建时间：2015-08-16
//
#include "pch.h"
#include "WICHelper.h"

using namespace NS_MEDIA;
using namespace WRL;

WICHelper::WICHelper()
{
	CreateWICComponent();
}

void WICHelper::CreateWICComponent()
{
	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC, 
		IID_PPV_ARGS(&wicFactory)));

}

void WICHelper::SaveTexture(ID3D11DeviceContext * d3dDeviceContext, ID3D11Texture2D * texture)
{
	//D3D11_MAPPED_SUBRESOURCE mappedRes;
	//ThrowIfFailed(d3dDeviceContext->Map(texture, 0, D3D11_MAP_READ, 0, &mappedRes));
	//finalizer fin([&] {d3dDeviceContext->Unmap(texture, 0);});
}