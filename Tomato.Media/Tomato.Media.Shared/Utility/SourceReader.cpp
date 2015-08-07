//
// Tomato Media
// 媒体源读取器
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#include "pch.h"
#include "SourceReader.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

SourceReader::SourceReader()
{
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

void SourceReader::SetMediaSource(IMFMediaSource* mediaSource)
{
	InitializeSourceReader(mediaSource);
	ConfigureSourceReader();
}

void SourceReader::OnEndOfStream()
{
	isActive = false;
}

void SourceReader::Start()
{
	isActive = true;
	drainSamples = false;

	PostSampleRequest();
}

void SourceReader::PostSampleRequest()
{
	ThrowIfFailed(sourceReader->ReadSample(MF_SOURCE_READER_ANY_STREAM, drainSamples ? MF_SOURCE_READER_CONTROLF_DRAIN : 0, nullptr, nullptr, nullptr, nullptr));
}

HRESULT SourceReader::OnFlush(DWORD dwStreamIndex)
{
	return S_OK;
}

HRESULT SourceReader::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent)
{
	return S_OK;
}

#if (WINVER >= _WIN32_WINNT_WIN7)

VideoSourceReader::VideoSourceReader(IDXGIAdapter* dxgiAdapter, ID3D11Device* d3dDevice)
	:d3dDevice(d3dDevice)
{
	InitializeDXGIDeviceManager(dxgiAdapter);
}

void VideoSourceReader::InitializeDXGIDeviceManager(IDXGIAdapter* dxgiAdapter)
{
	auto flags = static_cast<D3D11_CREATE_DEVICE_FLAG>(D3D11_CREATE_DEVICE_VIDEO_SUPPORT);
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
	if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION, &d3dDevice, nullptr, nullptr)))
	{
		ComPtr<ID3D10Multithread> d3dMultithread;
		ThrowIfFailed(d3dDevice.As(&d3dMultithread));
		d3dMultithread->SetMultithreadProtected(true);

		ThrowIfFailed(MFCreateDXGIDeviceManager(&resetToken, dxgiDeviceManager.ReleaseAndGetAddressOf()));
		ThrowIfFailed(dxgiDeviceManager->ResetDevice(d3dDevice.Get(), resetToken));
	}
}

void VideoSourceReader::ConfigureAttributes(IMFAttributes* attributes)
{
	if (dxgiDeviceManager)
	{
		ThrowIfFailed(attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxgiDeviceManager.Get()));
	}
}

void VideoSourceReader::SetStreamSelection(IMFSourceReader* sourceReader)
{
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, true));
	ConfigureOutputMediaType(sourceReader);
}

void VideoSourceReader::ConfigureOutputMediaType(IMFSourceReader* sourceReader)
{
	ComPtr<IMFMediaType> mediaType;

	ThrowIfFailed(MFCreateMediaType(&mediaType));
	ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
	ThrowIfFailed(sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaType.Get()));
	ThrowIfFailed(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, outputMT.ReleaseAndGetAddressOf()));

	ThrowIfFailed(MFGetAttributeSize(outputMT.Get(), MF_MT_FRAME_SIZE, &frameSize.Width, &frameSize.Height));
	UINT32 numerator, denominator;
	ThrowIfFailed(MFGetAttributeRatio(outputMT.Get(), MF_MT_FRAME_RATE, &numerator, &denominator));
	// 设置缓冲长度
	videoCacheSize = DefaultVideoCacheTime * numerator / denominator;
}

HRESULT VideoSourceReader::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	try
	{
		if (SUCCEEDED(hrStatus))
		{
			if (dwStreamFlags & MF_SOURCE_READERF_ERROR ||
				dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
				OnEndOfStream();

			if (pSample)
				DispatchIncomingSample(pSample);
		}
	}
	CATCH_ALL();

	return S_OK;
}

void VideoSourceReader::DispatchIncomingSample(IMFSample* pSample)
{
	{
		std::lock_guard<decltype(videoCacheMutex)> locker(videoCacheMutex);

		// 获取 Buffer 数量
		DWORD bufferCount;
		ThrowIfFailed(pSample->GetBufferCount(&bufferCount));

		ComPtr<ID3D11Texture2D> texture;
		for (DWORD i = 0; i < 1; i++)
		{
			ComPtr<IMFMediaBuffer> buffer;
			ThrowIfFailed(pSample->GetBufferByIndex(i, &buffer));

			ComPtr<IMFDXGIBuffer> dxgiBuffer;
			if (SUCCEEDED(buffer.As(&dxgiBuffer)))
				ThrowIfFailed(dxgiBuffer->GetResource(IID_PPV_ARGS(&texture)));
			else
			{
				ComPtr<IMF2DBuffer> buffer2d;
				ThrowIfFailed(buffer.As(&buffer2d));

				D3D11_TEXTURE2D_DESC desc{ 0 };
				desc.Width = frameSize.Width;
				desc.Height = frameSize.Height;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_NV12;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

				try
				{
					BYTE* base; LONG pitch;
					ThrowIfFailed(buffer2d->Lock2D(&base, &pitch));

					D3D11_SUBRESOURCE_DATA data{ base, static_cast<UINT>(pitch), 0 };

					ThrowIfFailed(d3dDevice->CreateTexture2D(&desc, &data, &texture));
					buffer2d->Unlock2D();
				}
				catch (...)
				{
					buffer2d->Unlock2D();
					throw;
				}
			}
		}

		if (texture)
		{
			REFERENCE_TIME time, duration;
			ThrowIfFailed(pSample->GetSampleTime(&time));
			ThrowIfFailed(pSample->GetSampleDuration(&duration));
			videoCache.emplace(texture, time, duration);
		}
	}
	PostSampleRequestIfNeed();
}

void VideoSourceReader::PostSampleRequestIfNeed()
{
	std::unique_lock<decltype(videoCacheStarveCondMutex)> locker(videoCacheStarveCondMutex);
	// 等待缓冲不足或停止读取
	videoCacheStarveCond.wait(locker, [=] {
		return videoCache.size() < videoCacheSize || !isActive;
	});

	if (isActive && videoCache.size() < videoCacheSize)
		PostSampleRequest();
}

bool VideoSourceReader::TryReadVideoSample(VideoSample& sample)
{
	if (!videoCache.empty())
	{
		std::lock_guard<decltype(videoCacheMutex)> locker(videoCacheMutex);

		if (!videoCache.empty())
		{
			sample = std::move(videoCache.front());
			videoCache.pop();
			return true;
		}
	}
	// 如果缓冲不足且未停止读取则发出提示
	if (isActive && videoCache.size() < videoCacheSize)
		videoCacheStarveCond.notify_one();
	return false;
}

#endif