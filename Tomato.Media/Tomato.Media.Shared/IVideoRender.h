//
// Tomato Media
// 视频渲染器接口
// 
// 作者：SunnyCase 
// 创建日期 2015-08-10
#pragma once
#include "common.h"
#include <ppltasks.h>
#include <mfidl.h>
#include <d3d11.h>

DEFINE_NS_MEDIA

struct Frame
{
	WRL::ComPtr<ID3D11ShaderResourceView> Luminance;
	WRL::ComPtr<ID3D11ShaderResourceView> Chrominance;

	Frame() {}

	Frame(WRL::ComPtr<ID3D11ShaderResourceView> luminance,
		WRL::ComPtr<ID3D11ShaderResourceView> chrominance)
		:Luminance(std::move(luminance)), Chrominance(std::move(chrominance))
	{

	}
};

///<summary>视频渲染器接口</summary>
struct DECLSPEC_UUID("E27E2141-0935-4CE7-8550-2B7B4DA8B5E9") IVideoRender : public IUnknown
{
	virtual concurrency::task<void> Initialize() = 0;
	virtual Frame CreateFrame(IMFSample* sample, UINT width, UINT height) = 0;
	virtual void RenderFrame(const Frame& frame) = 0;

#ifdef __cplusplus_winrt
	virtual void SetSurfaceImageSource(Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ imageSource, UINT width, UINT height) = 0;
#endif
};

END_NS_MEDIA

// {B6DF7A10-E9BF-435E-A079-C85672EC3B39}
EXTERN_GUID(MF_TM_VIDEORENDER_SERVICE,
	0xb6df7a10, 0xe9bf, 0x435e, 0xa0, 0x79, 0xc8, 0x56, 0x72, 0xec, 0x3b, 0x39);