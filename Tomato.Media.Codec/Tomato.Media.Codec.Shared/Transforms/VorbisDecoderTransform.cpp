//
// Tomato Media Codec
// Media Foundation Vorbis 解码 Transform
// 
// 作者：SunnyCase
// 创建时间：2015-09-14
#include "pch.h"
#include "VorbisDecoderTransform.h"
#include "../../include/MFRAII.h"
#include "constants.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#if (NTDDI_VERSION >= NTDDI_WIN8)
ActivatableClass(VorbisDecoderTransform);
#else
//CoCreatableClass(VorbisDecoderTransform);
#endif

const DecoderTransformRegisterInfo VorbisDecoderTransform::RegisterInfos[1] =
{
	{ MFAudioFormat_Vorbis, MFAudioFormat_Float },
};

VorbisDecoderTransform::VorbisDecoderTransform()
{
}

void VorbisDecoderTransform::OnValidateInputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find_if(std::begin(RegisterInfos), std::end(RegisterInfos), [&](auto&& info)
	{
		return info.InputSubType == subType;
	}) == std::end(RegisterInfos))
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

void VorbisDecoderTransform::OnValidateOutputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find_if(availableOutputTypes.begin(), availableOutputTypes.end(), [&](auto&& myType)
	{
		GUID mySubType;
		ThrowIfFailed(myType->GetGUID(MF_MT_SUBTYPE, &mySubType));
		return mySubType == subType;
	}) == availableOutputTypes.end())
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);

	UINT32 channels[2], sampleRate[2];
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels[0]));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate[0]));
	ThrowIfFailed(inputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels[1]));
	ThrowIfFailed(inputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate[1]));
	if(channels[0] != channels[1] || sampleRate[0] != sampleRate[1])
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

DWORD VorbisDecoderTransform::OnGetOutputFrameSize() const noexcept
{
	return bytesPerDecodecSample * decodedSamples;
}

WRL::ComPtr<IMFMediaType> VorbisDecoderTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeAvailableOutputTypes(type);
	return type;
}

WRL::ComPtr<IMFMediaType> VorbisDecoderTransform::OnSetOutputType(IMFMediaType * type)
{
	return type;
}

namespace
{
	ogg_packet CreatePacket(IMFSample* sample, BYTE* data, DWORD length)
	{
		ogg_packet packet;
		UINT32 bos, eos;
		UINT64 packetno, granulepos;
		ThrowIfFailed(sample->GetUINT32(MF_MT_OGG_PACKET_BOS, &bos));
		ThrowIfFailed(sample->GetUINT32(MF_MT_OGG_PACKET_EOS, &eos));
		ThrowIfFailed(sample->GetUINT64(MF_MT_OGG_PACKET_NO, &packetno));
		ThrowIfFailed(sample->GetUINT64(MF_MT_OGG_PACKET_GRANULEPOS, &granulepos));

		packet.bytes = long(length);
		packet.b_o_s = long(bos);
		packet.e_o_s = long(eos);
		packet.granulepos = ogg_int64_t(granulepos);
		packet.packet = data;
		packet.packetno = ogg_int64_t(packetno);
		return packet;
	}
}

bool VorbisDecoderTransform::OnReceiveInput(IMFSample * sample)
{
	ComPtr<IMFMediaBuffer> buffer;
	ThrowIfFailed(sample->ConvertToContiguousBuffer(buffer.GetAddressOf()));

	BYTE* data; DWORD length;
	Core::MFBufferLocker locker(buffer.Get());
	locker.Lock(data, nullptr, &length);
	auto packet = CreatePacket(sample, data, length);

	if (packet.packetno == 0)
		vorbis_synthesis_restart(&vorbisDspState);

	// Header
	if (vorbisState == VorbisState::Header)
	{
		auto ret = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &packet);
		// 遇到了 Body
		if (ret != 0 && vorbisInfo.codec_setup)
		{
			vorbisDspState.assign(vorbisInfo);
			vorbisBlock.assign(vorbisDspState);
			bytesPerDecodecSample = vorbisInfo.channels * 4;	// 32bit Float

			vorbisState = VorbisState::Body;
			return FeedBodyPacket(packet);
		}
		// 忽略错误
		return false;
	}
	// Body
	return FeedBodyPacket(packet);
}

void VorbisDecoderTransform::OnProduceOutput(MFT_OUTPUT_DATA_BUFFER & output)
{
	if (!output.pSample) ThrowIfFailed(E_INVALIDARG);
	ComPtr<IMFMediaBuffer> buffer;
	ThrowIfFailed(output.pSample->GetBufferByIndex(0, buffer.GetAddressOf()));
	BYTE* data; DWORD maxLength;
	DWORD cntLength = 0;
	{
		Core::MFBufferLocker locker(buffer.Get());
		locker.Lock(data, &maxLength, nullptr);
		if (maxLength < OnGetOutputFrameSize()) ThrowIfFailed(E_INVALIDARG);
		memset(data, 0, maxLength);

		cntLength = FillFloatFrame(buffer.Get(), data, maxLength);
	}
	ThrowIfFailed(buffer->SetCurrentLength(cntLength));
	auto duration = MFTIME(decodedSamples * 1e7 / vorbisInfo.rate);
	decodedSamples = 0;
}

WRL::ComPtr<IMFMediaType> VorbisDecoderTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}

void VorbisDecoderTransform::InitializeAvailableOutputTypes(IMFMediaType* inputType)
{
	availableOutputTypes.clear();

	UINT32 channels, sampleRate;
	ThrowIfFailed(inputType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels));
	ThrowIfFailed(inputType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate));

	WAVEFORMATEX waveFormat = { 0 };
	waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	waveFormat.nChannels = channels;
	waveFormat.nSamplesPerSec = sampleRate;
	waveFormat.wBitsPerSample = 4 * 8;
	waveFormat.nBlockAlign = 4 * channels;
	waveFormat.nAvgBytesPerSec = sampleRate * waveFormat.nBlockAlign;

	ComPtr<IMFMediaType> outputMediaType;
	ThrowIfFailed(MFCreateMediaType(&outputMediaType));
	ThrowIfFailed(MFInitMediaTypeFromWaveFormatEx(outputMediaType.Get(), &waveFormat, sizeof(waveFormat)));

	availableOutputTypes.emplace_back(std::move(outputMediaType));
}

void VorbisDecoderTransform::InitializeDecoder()
{
	vorbisBlock.reset();
	vorbisDspState.reset();
	vorbisComment.reset();
	vorbisInfo.reset();
	vorbisState = VorbisState::Header;
}

bool VorbisDecoderTransform::FeedBodyPacket(ogg_packet & packet)
{
	auto ret = vorbis_synthesis(&vorbisBlock, &packet);
	// 提交 packet 成功
	if (ret == 0)
	{
		ret = vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock);
		// 提交 block 成功
		if (ret == 0)
		{
			decodedSamples = vorbis_synthesis_pcmout(&vorbisDspState, nullptr);
			return decodedSamples != 0;
		}
	}
	// 忽略其它错误
	return false;
}

DWORD VorbisDecoderTransform::FillFloatFrame(IMFMediaBuffer* buffer, BYTE * data, DWORD maxLength)
{
	auto cntData = reinterpret_cast<float*>(data);
	float** channelsSrc = nullptr;
	auto availSamples = vorbis_synthesis_pcmout(&vorbisDspState, &channelsSrc);

	if (availSamples && channelsSrc)
	{
		const auto samplesToRead = size_t(availSamples);
		const auto channels = size_t(vorbisInfo.channels);
		if (maxLength < samplesToRead * channels * 4)
			ThrowIfFailed(E_INVALIDARG);

		for (size_t i = 0; i < size_t(availSamples); i++)
			for (size_t channel = 0; channel < channels; channel++)
				*cntData++ = channelsSrc[channel][i];

		ThrowIfNot(vorbis_synthesis_read(&vorbisDspState, availSamples) == 0,
			L"Unexpected decode error.");
	}

	return DWORD(reinterpret_cast<BYTE*>(cntData) - data);
}

void VorbisDecoderTransform::BeginStreaming()
{
}