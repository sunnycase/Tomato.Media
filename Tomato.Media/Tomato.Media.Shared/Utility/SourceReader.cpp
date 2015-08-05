//
// Tomato Media
// 媒体源读取器
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#include "pch.h"
#include "SourceReader.h"
#include "..\SourceReader.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

SourceReader::SourceReader(IMFMediaSource* mediaSource)
{
	InitializeSourceReader(mediaSource);
}

void SourceReader::InitializeSourceReader(IMFMediaSource* mediaSource)
{
	ComPtr<IMFAttributes> attributes;
	ThrowIfFailed(MFCreateAttributes(&attributes, 3));
	// 设置 Callback
	ThrowIfFailed(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this));
#if (WINVER >= _WIN32_WINNT_WIN8)
	ThrowIfFailed(attributes->SetString(MF_READWRITE_MMCSS_CLASS_AUDIO, L"Audio"));
	ThrowIfFailed(attributes->SetUINT32(MF_READWRITE_MMCSS_PRIORITY_AUDIO, 4));
#endif

	ConfigureAttributes(attributes.Get());
	ThrowIfFailed(MFCreateSourceReaderFromMediaSource(mediaSource, attributes.Get(), sourceReader.ReleaseAndGetAddressOf()));
}

void SourceReader::ConfigureSourceReader()
{
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false));
	SetStreamSelection(sourceReader.Get());
}

void SourceReader::Start()
{

}

HRESULT SourceReader::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	return E_NOTIMPL;
}

HRESULT SourceReader::OnFlush(DWORD dwStreamIndex)
{
	return E_NOTIMPL;
}

HRESULT SourceReader::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent)
{
	return E_NOTIMPL;
}

#if (WINVER >= _WIN32_WINNT_WIN8)

VideoSourceReader::VideoSourceReader(IMFMediaSource* mediaSource, ID3D11Device* d3dDevice)
	:SourceReader(mediaSource)
{
	ThrowIfFailed(MFCreateDXGIDeviceManager(&resetToken, dxgiDeviceManager.ReleaseAndGetAddressOf()));
	ThrowIfFailed(dxgiDeviceManager->ResetDevice(d3dDevice, resetToken));
}

void VideoSourceReader::ConfigureAttributes(IMFAttributes* attributes)
{
	ThrowIfFailed(attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxgiDeviceManager.Get()));
}

void VideoSourceReader::SetStreamSelection(IMFSourceReader* sourceReader)
{
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, true));
}

#endif