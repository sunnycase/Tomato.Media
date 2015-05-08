//
// Tomato Media
// Media Foundation 媒体源读取器
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "MFSourceReader.h"
#include "Utilities/mfhelpers.hpp"
#include "Transforms/MFTransforms/LibAVMFTransform.h"
#include "Utilities/MFByteStreamHandlers/LibAVByteStreamHandler.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace
{
	struct MFTRegistry
	{
		MFTRegistry()
		{
			THROW_IF_FAILED(MFStartup(MF_SDK_VERSION, MFSTARTUP_LITE));
			RegisterMFTs();
		}

		~MFTRegistry()
		{
			MFShutdown();
		}

		void RegisterMFTs()
		{
			static auto extensionManager = ref new Windows::Media::MediaExtensionManager();

			extensionManager->RegisterByteStreamHandler(ref new Platform::String(
				LibAVByteStreamHandler::InternalGetRuntimeClassName()), ".ape", "audio/x-ape");
			extensionManager->RegisterByteStreamHandler(ref new Platform::String(
				LibAVByteStreamHandler::InternalGetRuntimeClassName()), ".flac", "audio/flac");

			extensionManager->RegisterAudioDecoder(ref new Platform::String(
				LibAVMFTransform::InternalGetRuntimeClassName()),
				KSDATAFORMAT_SUBTYPE_LIBAV, MFAudioFormat_PCM);
		}
	};
}


void NS_TOMATO_MEDIA::RegisterMFTs()
{
	static MFTRegistry reg;
}

MFSourceReader::MFSourceReader(IMediaSourceIntern* mediaSource)
	:sourceReaderCallback(Make<MFSourceReaderCallback>())
{
	RegisterMFTs();

	sourceReaderCallback->SetReadSampleCallback(std::bind(
		&MFSourceReader::OnSampleRead, this, std::placeholders::_1));
	Initialize(mediaSource->CreateMFByteStream());
}

void MFSourceReader::Start(int64_t hns)
{
	if (readerState == SourceReaderState::Ready ||
		readerState == SourceReaderState::Stopped ||
		readerState == SourceReaderState::Ended)
	{
		if (hns != -1) SetCurrentPosition(hns);
		readerState = SourceReaderState::PreRoll;
		sourceReaderCallback->Start();
		sourceReaderCallback->BeginReadSample(sourceReader.Get());
	}
}

task<void> MFSourceReader::StopAsync()
{
	if (readerState == SourceReaderState::Playing ||
		readerState == SourceReaderState::Ended)
	{
		// 停止读取任务
		readerState = SourceReaderState::Stopped;

		sourceReaderCallback->Stop();
		requestEvent.set();
		return sourceReaderCallback->FlushAsync(sourceReader.Get())
			.then([=]
		{
			// 清理缓冲
			decodedBuffer.clear();
			firstSample = true;
			bufferStartPosition = 0;
		});
	}
	return task_from_result();
}

size_t MFSourceReader::Read(byte * buffer, size_t bufferSize)
{
	if (!IsPreRollFilled())
		requestEvent.set();

	// 缓冲时、媒体结束不返回数据
	if (readerState == SourceReaderState::PreRoll)
		return 0;
	if (decodedBuffer.tell_not_get() == 0)
	{
		// 读取结束
		if (readerState == SourceReaderState::Stopped ||
			readerState == SourceReaderState::Ended) return 0;
		else
			return 0;
	}
	else
	{
		// 最小为一个设备周期
		if (bufferSize < bytesPerPeriodLength) return 0;
		return decodedBuffer.read(buffer, bufferSize);
	}
}

void MFSourceReader::SetAudioFormat(const WAVEFORMATEX * format, uint32_t framesPerPeriod)
{
	InitializeOutputMediaType(format);
	ConfigureSourceReader();
	bytesPerPeriodLength = framesPerPeriod * outputFormat->nBlockAlign;
	decodedBuffer.init(format->nAvgBytesPerSec * 10);

	readerState = SourceReaderState::Ready;
}

void MFSourceReader::SetCurrentPosition(int64_t hns)
{
	PROPVARIANT positionVar;
	PropVariantInit(&positionVar);
	positionVar.vt = VT_I8;
	positionVar.hVal.QuadPart = hns;
	THROW_IF_FAILED(sourceReader->SetCurrentPosition(GUID_NULL, positionVar));
}

void MFSourceReader::InitializeOutputMediaType(const WAVEFORMATEX * outputFormat)
{
	THROW_IF_FAILED(MFCreateMediaType(outputMT.ReleaseAndGetAddressOf()));
	THROW_IF_FAILED(MFInitMediaTypeFromWaveFormatEx(outputMT.Get(),
		outputFormat, sizeof(WAVEFORMATEX) + outputFormat->cbSize));
}

void MFSourceReader::Initialize(ComPtr<IMFByteStream>&& byteStream)
{
	readerState = SourceReaderState::Initializing;
	ComPtr<IMFAttributes> attributes;

	THROW_IF_FAILED(MFCreateAttributes(&attributes, 3));
	// Specify Source Reader Attributes
	THROW_IF_FAILED(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,
		static_cast<IMFSourceReaderCallback*>(sourceReaderCallback.Get())));
	THROW_IF_FAILED(attributes->SetString(MF_READWRITE_MMCSS_CLASS_AUDIO, L"Pro Audio"));
	THROW_IF_FAILED(attributes->SetUINT32(MF_READWRITE_MMCSS_PRIORITY_AUDIO, 0));

	ComPtr<IMFSourceReader> pSrcReader;
	// Create source reader
	THROW_IF_FAILED(MFCreateSourceReaderFromByteStream(byteStream.Get(), attributes.Get(), &pSrcReader));
	THROW_IF_FAILED(pSrcReader.As(&sourceReader));
}

void MFSourceReader::ConfigureSourceReader()
{
	// 只选择第一个音频流
	THROW_IF_FAILED(sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false));
	THROW_IF_FAILED(sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true));

	// 配置SourceReader的媒体类型（自动加载解码器）
	THROW_IF_FAILED(sourceReader->SetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, outputMT.Get()));

	// 获取解码后的媒体类型
	THROW_IF_FAILED(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		outputMT.ReleaseAndGetAddressOf()));

	WAVEFORMATEX *pwfx = nullptr;
	UINT32 cbFormat = 0;
	// 获取解码后的波形格式
	THROW_IF_FAILED(MFCreateWaveFormatExFromMFMediaType(outputMT.Get(), &pwfx, &cbFormat));
	outputFormat.reset(pwfx);
}

void MFSourceReader::OnSampleRead(ReadSampleResult result)
{
	if ((readerState != SourceReaderState::Playing) &&
		(readerState != SourceReaderState::PreRoll))
		return;

	if ((result.streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) ||
		(result.streamFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED))
	{
		readerState = SourceReaderState::Ended;
		return;
	}

	EnqueueSample(result.sample);
	// Pre-roll PREROLL_DURATION seconds worth of data
	if (readerState == SourceReaderState::PreRoll)
	{
		if (IsPreRollFilled())
		{
			// Once Pre-roll is filled, audio endpoint will stop rendering silence and start
			// picking up data from the queue
			readerState = SourceReaderState::Playing;
		}
	}
	// 停止状态下不再继续读取
	if (readerState != SourceReaderState::Stopped &&
		readerState != SourceReaderState::Ended)
	{
		requestEvent.reset();
		// Call ReadSample for next asynchronous sample event
		sourceReaderCallback->BeginReadSample(sourceReader.Get());
	}
}

void MFSourceReader::EnqueueSample(ComPtr<IMFSample>& sample)
{
	ComPtr<IMFMediaBuffer> mediaBuffer;
	BYTE* audioData = nullptr;
	DWORD audioDataLength = 0;

	// Since we are storing the raw byte data, convert this to a single buffer
	THROW_IF_FAILED(sample->ConvertToContiguousBuffer(&mediaBuffer));

	// 第一个样本则记录开始时间
	if (firstSample)
	{
		THROW_IF_FAILED(sample->GetSampleTime(&bufferStartPosition));
		firstSample = false;
	}

	// Lock the sample
	mfbuffer_locker locker(mediaBuffer.Get());
	THROW_IF_FAILED(locker.lock(&audioData, nullptr, &audioDataLength));

	THROW_IF_NOT(audioDataLength != 0, "无法识别的音频数据");

	auto read = decodedBuffer.write(audioData, audioDataLength);
	// 没有写完，需要等待读取
	while (read < audioDataLength)
	{
		audioData += read;
		audioDataLength -= read;
		if (requestEvent.wait() != 0 || readerState == SourceReaderState::Stopped ||
			readerState == SourceReaderState::Ended)return;
		read = decodedBuffer.write(audioData, audioDataLength);
	}
}

bool MFSourceReader::IsPreRollFilled()
{
	auto desired = outputFormat->nAvgBytesPerSec * PREROLL_DURATION_SEC;

	return decodedBuffer.tell_not_get() >= desired;
}

void MFSourceReader::NotifyReadSample()
{
}

MFSourceReaderCallback::MFSourceReaderCallback()
{
}

STDMETHODIMP MFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) noexcept
{
	try
	{
		THROW_IF_FAILED(hrStatus);
		auto& callback = readSampleCallback;
		if (callback)
			callback({ pSample, dwStreamFlags });
	}
	CATCH_ALL();
	return S_OK;
}

STDMETHODIMP MFSourceReaderCallback::OnFlush(DWORD dwStreamIndex) noexcept
{
	flushEvent.set();
	return S_OK;
}

task<void> MFSourceReaderCallback::FlushAsync(IMFSourceReaderEx* sourceReader)
{
	flushEvent = decltype(flushEvent)();
	THROW_IF_FAILED(sourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS));
	return create_task(flushEvent);
}

STDMETHODIMP MFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) noexcept
{
	return S_OK;
}

void MFSourceReaderCallback::SetReadSampleCallback(std::function<void(ReadSampleResult)>&& readSampleCallback)
{
	this->readSampleCallback = std::move(readSampleCallback);
}

void MFSourceReaderCallback::BeginReadSample(IMFSourceReaderEx* sourceReader)
{
	if (!stopped)
	{
		THROW_IF_FAILED(sourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0, nullptr, nullptr, nullptr, nullptr));
	}
}

void MFSourceReaderCallback::Stop()
{
	stopped = true;
}

void MFSourceReaderCallback::Start()
{
	stopped = false;
}

MEDIA_CORE_API std::unique_ptr<ISourceReader> __stdcall NS_TOMATO_MEDIA::CreateMFSourceReader(
	IMediaSource* mediaSource)
{
	return std::make_unique<MFSourceReader>(reinterpret_cast<IMediaSourceIntern*>(mediaSource));
}