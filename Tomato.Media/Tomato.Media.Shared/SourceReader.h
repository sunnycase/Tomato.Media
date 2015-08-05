//
// Tomato Media
// 媒体源读取器
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#pragma once
#include "common.h"
#include <mfreadwrite.h>
#include <d3d11.h>

DEFINE_NS_MEDIA

class SourceReader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::RuntimeClassType::ClassicCom>, IMFSourceReaderCallback>
{
public:
	SourceReader(IMFMediaSource* mediaSource);

	void Start();

	// 通过 RuntimeClass 继承
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) override;
	STDMETHODIMP OnFlush(DWORD dwStreamIndex) override;
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) override;
protected:
	virtual void ConfigureAttributes(IMFAttributes* attributes) {};
	virtual void SetStreamSelection(IMFSourceReader* sourceReader) = 0;
private:
	void InitializeSourceReader(IMFMediaSource* mediaSource);
	void ConfigureSourceReader();
private:
	WRL::ComPtr<IMFSourceReader> sourceReader;
};

#if (WINVER >= _WIN32_WINNT_WIN8)

class VideoSourceReader : public SourceReader
{
public:
	VideoSourceReader(IMFMediaSource* mediaSource, ID3D11Device* d3dDevice);
protected:
	virtual void ConfigureAttributes(IMFAttributes* attributes) override;
	virtual void SetStreamSelection(IMFSourceReader* sourceReader) override;
private:
	UINT resetToken;
	WRL::ComPtr<IMFDXGIDeviceManager> dxgiDeviceManager;
};

#endif

END_NS_MEDIA