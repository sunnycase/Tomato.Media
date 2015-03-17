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

struct MFTRegistry
{
	MFTRegistry()
	{
		av_log_set_callback(AVLogCallback);
		av_register_all();
		avcodec_register_all();
		RegisterMFTs();
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

	static void AVLogCallback(void* avcl, int level, const char* fmt, va_list vargs)
	{
		char buffer[INT16_MAX];
		vsprintf_s(buffer, fmt, vargs);
		OutputDebugStringA(buffer);
	}
};

MFSourceReader::MFSourceReader(IMediaSourceIntern* mediaSource)
	:sourceReaderCallback(Make<MFSourceReaderCallback>())
{
	sourceReaderCallback->SetReadSampleCallback(std::bind(
		&MFSourceReader::OnSampleRead, this, std::placeholders::_1));
	Initialize(mediaSource->CreateMFByteStream());
}

void MFSourceReader::Start()
{
	if (readerState == SourceReaderState::Ready ||
		readerState == SourceReaderState::Stopped)
	{
		readerState = SourceReaderState::PreRoll;
		sourceReaderCallback->BeginReadSample(sourceReader.Get());
	}
}

size_t MFSourceReader::Read(byte * buffer, size_t bufferSize)
{
	std::lock_guard<decltype(stateMutex)> locker(stateMutex);

	// 缓冲时不返回数据
	if (readerState == SourceReaderState::PreRoll)
		return 0;
	if (decodedBuffer.tell_not_get() == 0)
	{
		// 读取结束
		if (readerState == SourceReaderState::Stopped)
			return 0;
		else
			return 0;
	}
	else
	{
		// 数据必须按帧对齐
		// 最小读取数
		auto to_read = std::min(bytesPerPeriodLength, decodedBuffer.tell_not_get());
		// 缓冲区剩余空间不足最小读取数，等待下次调用
		if (bufferSize < to_read)
			return 0;
		else
		{
			return decodedBuffer.read(buffer, to_read);
		}
	}
}

void MFSourceReader::SetAudioFormat(const WAVEFORMATEX * format, uint32_t framesPerPeriod)
{
	InitializeOutputMediaType(format);
	ConfigureSourceReader();
	bytesPerPeriodLength = framesPerPeriod * outputFormat->nBlockAlign;
	decodedBuffer.init(bytesPerPeriodLength);

	readerState = SourceReaderState::Ready;
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
		(readerState != SourceReaderState::PreRoll) &&
		(readerState != SourceReaderState::Paused))
		return;

	if ((result.streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) ||
		(result.streamFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED))
	{
		readerState = SourceReaderState::Stopped;
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
test1:
	// 暂停状态下不再继续读取
	if (readerState != SourceReaderState::Paused)
	{
		if (IsPreRollFilled())
		{
			Sleep(100);
			goto test1;
		}
		// Call ReadSample for next asynchronous sample event
		sourceReaderCallback->BeginReadSample(sourceReader.Get());
	}
}

void MFSourceReader::EnqueueSample(ComPtr<IMFSample>& sample)
{
	std::lock_guard<decltype(stateMutex)> locker2(stateMutex);

	ComPtr<IMFMediaBuffer> mediaBuffer;
	BYTE* audioData = nullptr;
	DWORD audioDataLength = 0;

	// Since we are storing the raw byte data, convert this to a single buffer
	THROW_IF_FAILED(sample->ConvertToContiguousBuffer(&mediaBuffer));
	// Lock the sample
	mfbuffer_locker locker(mediaBuffer.Get());
	THROW_IF_FAILED(locker.lock(&audioData, nullptr, &audioDataLength));

	THROW_IF_NOT(audioDataLength != 0, "无法识别的音频数据");

	decodedBuffer.write(audioData, audioDataLength);
}

bool MFSourceReader::IsPreRollFilled()
{
	auto desired = outputFormat->nAvgBytesPerSec * PREROLL_DURATION_SEC;

	return decodedBuffer.tell_not_get() >= desired;
}

MFSourceReaderCallback::MFSourceReaderCallback()
{
}

STDMETHODIMP MFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) noexcept
{
	try
	{
		std::lock_guard<decltype(readMutex)> locker(readMutex);
		THROW_IF_FAILED(hrStatus);
		if (readSampleCallback)
			readSampleCallback({ pSample, dwStreamFlags });
	}
	CATCH_ALL();
	return S_OK;
}

STDMETHODIMP MFSourceReaderCallback::OnFlush(DWORD dwStreamIndex) noexcept
{
	return E_NOTIMPL;
}

STDMETHODIMP MFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) noexcept
{
	return E_NOTIMPL;
}

void MFSourceReaderCallback::SetReadSampleCallback(std::function<void(ReadSampleResult)>&& readSampleCallback)
{
	this->readSampleCallback = std::move(readSampleCallback);
}

void MFSourceReaderCallback::BeginReadSample(IMFSourceReaderEx* sourceReader)
{
	std::lock_guard<decltype(readMutex)> locker(readMutex);

	THROW_IF_FAILED(sourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		0, nullptr, nullptr, nullptr, nullptr));
}

MEDIA_CORE_API std::unique_ptr<ISourceReader> __stdcall NS_TOMATO_MEDIA::CreateMFSourceReader(
	IMediaSource* mediaSource)
{
	static MFTRegistry mftReg;

	return std::make_unique<MFSourceReader>(reinterpret_cast<IMediaSourceIntern*>(mediaSource));
}