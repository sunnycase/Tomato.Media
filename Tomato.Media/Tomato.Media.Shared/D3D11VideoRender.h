//
// Tomato Media
// D3D11 视频渲染器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-10
#pragma once
#include "IVideoRender.h"
#include <d3d11.h>
#include <windows.ui.xaml.media.dxinterop.h>

DEFINE_NS_MEDIA

///<summary>视频渲染器</summary>
class D3D11VideoRender : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IVideoRender>
{
public:
	D3D11VideoRender();

	virtual void Initialize() override;
	virtual WRL::ComPtr<ID3D11Texture2D> CreateFrame(IMFSample* sample, UINT width, UINT height) override;
	virtual void RenderFrame(ID3D11Texture2D* texture) override;

#ifdef __cplusplus_winrt
	virtual void SetSurfaceImageSource(Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ imageSource, UINT width, UINT height);
#endif
private:
	void CreateDeviceResources();
	void RenderFrameToSurfaceImageSource(ID3D11Texture2D* texture);
private:
	WRL::ComPtr<ID3D11Device> d3dDevice;
	WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	D3D_FEATURE_LEVEL featureLevel;
#ifdef __cplusplus_winrt
	Windows::UI::Core::CoreDispatcher^ sisDispatcher;
	WRL::ComPtr<ISurfaceImageSourceNative> sisNative;
	UINT sisWidth, sisHeight;
#endif
};

END_NS_MEDIA