//
// Tomato Media
// 视频引擎
// 
// 作者：SunnyCase 
// 创建日期 2015-08-06
#pragma once
#include "common.h"
#include "SourceReader.h"
#include <mfidl.h>
#include <d2d1_1.h>

DEFINE_NS_MEDIA

///<summary>视频引擎</summary>
class VideoEngine
{
public:
	VideoEngine();

	void SetMediaSource(IMFMediaSource* mediaSource);

	void Play();

	DEFINE_PROPERTY_GET(FrameSize, USIZE);
	USIZE get_FrameSize() const;

	DEFINE_PROPERTY_GET(D2dDevice, ID2D1Device*);
	ID2D1Device* get_D2dDevice() const { return d2dDevice.Get(); }
private:
	WRL::ComPtr<IDXGIAdapter> InitializeD2dDevice();
	void InitializeSourceReader(IDXGIAdapter* dxgiAdapter);
	void ConfigureSourceReader(IMFMediaSource* mediaSource);
	void InitializeSinkWriter();
private:
	WRL::ComPtr<VideoSourceReader> videoSourceReader;
	WRL::ComPtr<ID3D11Device> d3dDevice;
	WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	WRL::ComPtr<ID2D1Device> d2dDevice;
	WRL::ComPtr<IMFSinkWriter> sinkWriter;

	D3D_FEATURE_LEVEL featureLevel;
};

END_NS_MEDIA