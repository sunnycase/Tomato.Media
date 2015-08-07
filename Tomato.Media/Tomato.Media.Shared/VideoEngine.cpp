//
// Tomato Media
// 视频引擎
// 
// 作者：SunnyCase 
// 创建日期 2015-08-06
#include "pch.h"
#include "VideoEngine.h"
#include "MediaSink.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

VideoEngine::VideoEngine()
{
	auto dxgiAdapter = InitializeD2dDevice();
	InitializeSourceReader(dxgiAdapter.Get());
}

void VideoEngine::SetMediaSource(IMFMediaSource* mediaSource)
{
	ConfigureSourceReader(mediaSource);
	InitializeSinkWriter();
}

WRL::ComPtr<IDXGIAdapter> VideoEngine::InitializeD2dDevice()
{
	// 创建 D3D 设备
	D3D11_CREATE_DEVICE_FLAG flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		d3dDevice.ReleaseAndGetAddressOf(), &featureLevel, d3dDeviceContext.ReleaseAndGetAddressOf()));

	// 创建 D3D 设备
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(d3dDevice.As(&dxgiDevice));

	const D2D1_CREATION_PROPERTIES properties{
		D2D1_THREADING_MODE_SINGLE_THREADED,
		D2D1_DEBUG_LEVEL_NONE,
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE
	};
	ThrowIfFailed(D2D1CreateDevice(dxgiDevice.Get(), properties, d2dDevice.ReleaseAndGetAddressOf()));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.ReleaseAndGetAddressOf()));
	return dxgiAdapter;
}

void VideoEngine::InitializeSourceReader(IDXGIAdapter* dxgiAdapter)
{
	videoSourceReader = Make<VideoSourceReader>(dxgiAdapter, d3dDevice.Get());
}

void VideoEngine::ConfigureSourceReader(IMFMediaSource* mediaSource)
{
	videoSourceReader->SetMediaSource(mediaSource);
}

void VideoEngine::Play()
{
	if (!videoSourceReader)
		ThrowIfFailed(E_NOT_VALID_STATE);

	videoSourceReader->Start();

	/*while (true)
	{
		sinkWriter->WriteSample()
	}*/
}

USIZE VideoEngine::get_FrameSize() const
{
	return videoSourceReader->FrameSize;
}

void VideoEngine::InitializeSinkWriter()
{
	auto mediaSink = Make<MediaSink>();
	ComPtr<IMFStreamSink> streamSink;

	ThrowIfFailed(MFCreateSinkWriterFromMediaSink(mediaSink.Get(), nullptr, sinkWriter.ReleaseAndGetAddressOf()));

	DWORD streamIndex;
	ThrowIfFailed(sinkWriter->AddStream(videoSourceReader->GetOutputMediaType(), &streamIndex));
	ThrowIfFailed(sinkWriter->SetInputMediaType(streamIndex, videoSourceReader->GetOutputMediaType(), nullptr));
}