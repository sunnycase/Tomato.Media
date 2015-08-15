//
// Tomato Media
// D3D11 视频渲染器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-10
#pragma once
#include "IVideoRender.h"
#include <atomic>
#include <d3d11.h>

#ifdef __cplusplus_winrt
#include <windows.ui.xaml.media.dxinterop.h>
#endif

DEFINE_NS_MEDIA

///<summary>视频渲染器</summary>
class D3D11VideoRender : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IVideoRender>
{
public:
	D3D11VideoRender();

	virtual concurrency::task<void> Initialize() override;
	virtual Frame CreateFrame(IMFSample* sample, UINT width, UINT height) override;
	virtual void RenderFrame(const Frame& frame) override;

#ifdef __cplusplus_winrt
	virtual void SetSurfaceImageSource(Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^ imageSource, UINT width, UINT height);
#endif
private:
	concurrency::task<void> InitializeCore();
	concurrency::task<void> CreateDeviceResources();
	void ConfigureDeviceState();
	void CreateVideoLayerResources(float width, float height);
	void DrawVideoLayer(ID3D11RenderTargetView** target, const Frame& frame);
	void RenderFrameToSurfaceImageSource(const Frame& frame);
private:
	WRL::ComPtr<ID3D11Device> d3dDevice;
	WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	D3D_FEATURE_LEVEL featureLevel;
	std::atomic_bool initStarted = false;
	concurrency::task<void> initTask;
	WRL::ComPtr<ID3D11Buffer> videoLayerVB, videoLayerIB;
#ifdef __cplusplus_winrt
	Windows::UI::Core::CoreDispatcher^ sisDispatcher;
	WRL::ComPtr<ISurfaceImageSourceNative> sisNative;
	UINT sisWidth, sisHeight;
#endif
};

END_NS_MEDIA