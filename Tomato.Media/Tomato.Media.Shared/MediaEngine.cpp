//
// Tomato Media
// Media Engine
// 作者：SunnyCase
// 创建时间：2015-08-06
//
#include "pch.h"
#include "MediaEngine.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

#if (WINVER >= _WIN32_WINNT_WIN8)

namespace
{
	class MediaEngineExtension : public RuntimeClass<RuntimeClassFlags<RuntimeClassType::ClassicCom>, IMFMediaEngineExtension>
	{
	public:
		MediaEngineExtension(IMFMediaSource* mediaSource)
		{

		}
	private:

	};
}

MediaEngine::MediaEngine()
{
	std::weak_ptr<MediaEngine> weak(shared_from_this());
	engineNotify = Make<MediaEngineNotify>([weak](DWORD event) {
		auto me = weak.lock();
		if (me) me->OnMediaEngineEventNotify(event);
	});

	InitializeMediaEngine();
}

void MediaEngine::InitializeMediaEngine()
{
	ComPtr<IMFMediaEngineClassFactory> factory;
	ThrowIfFailed(CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&factory)));

	ComPtr<IMFAttributes> attributes;
	ThrowIfFailed(MFCreateAttributes(&attributes, 3));
	ThrowIfFailed(attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, engineNotify.Get()));
	ConfigureFactoryAttributes(attributes.Get());

	ComPtr<IMFMediaEngine> tmpMediaEngine;
	ThrowIfFailed(factory->CreateInstance(0, attributes.Get(), &tmpMediaEngine));
	ThrowIfFailed(tmpMediaEngine.As(&mediaEngine));
}

void MediaEngine::OnMediaEngineEventNotify(DWORD event)
{

}

VideoMediaEngine::VideoMediaEngine()
{

}

std::shared_ptr<VideoMediaEngine> VideoMediaEngine::Make()
{
	return std::shared_ptr<VideoMediaEngine>(new VideoMediaEngine());
}

ComPtr<IMFDXGIDeviceManager> VideoMediaEngine::InitializeDXGIDeviceManager()
{
	auto flags = static_cast<D3D11_CREATE_DEVICE_FLAG>(D3D11_CREATE_DEVICE_VIDEO_SUPPORT | D3D11_CREATE_DEVICE_BGRA_SUPPORT);
	const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	ComPtr<ID3D11Device> d3dDevice;
	if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION, &d3dDevice, nullptr, nullptr)))
	{
		ComPtr<IMFDXGIDeviceManager> dxgiDeviceManager;
		ThrowIfFailed(MFCreateDXGIDeviceManager(&resetToken, &dxgiDeviceManager));
		ThrowIfFailed(dxgiDeviceManager->ResetDevice(d3dDevice.Get(), resetToken));

		return dxgiDeviceManager;
	}
	return nullptr;
}

void VideoMediaEngine::ConfigureFactoryAttributes(IMFAttributes* attributes)
{
	auto dxgiDeviceManager(InitializeDXGIDeviceManager());
	ThrowIfFailed(attributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, dxgiDeviceManager.Get()));
}

#endif