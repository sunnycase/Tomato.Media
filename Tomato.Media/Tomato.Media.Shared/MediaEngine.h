//
// Tomato Media
// Media Engine
// 作者：SunnyCase
// 创建时间：2015-08-06
//
#pragma once
#include "common.h"
#include "Utility/MediaEngineNotify.h"

DEFINE_NS_MEDIA

#if (WINVER >= _WIN32_WINNT_WIN8)

class MediaEngine : public std::enable_shared_from_this<MediaEngine>
{
protected:
	MediaEngine();

	virtual void ConfigureFactoryAttributes(IMFAttributes* attributes) {}
private:
	void OnMediaEngineEventNotify(DWORD event);
	void InitializeMediaEngine();
private:
	WRL::ComPtr<MediaEngineNotify> engineNotify;
	WRL::ComPtr<IMFMediaEngineEx> mediaEngine;
};

class VideoMediaEngine : public MediaEngine
{
public:
	static std::shared_ptr<VideoMediaEngine> Make();
protected:
	VideoMediaEngine();

	virtual void ConfigureFactoryAttributes(IMFAttributes* attributes) override;
private:
	WRL::ComPtr<IMFDXGIDeviceManager> InitializeDXGIDeviceManager();
private:
	UINT resetToken;
};

#endif

END_NS_MEDIA
