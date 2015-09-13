//
// Tomato Media Codec
// Media Foundation Theora 解码 Transform
// 
// 作者：SunnyCase
// 创建时间：2015-09-08
#include "pch.h"
#include "TheoraDecoderTransform.h"
#include "../../include/MFRAII.h"
#include "constants.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#if (NTDDI_VERSION >= NTDDI_WIN8)
ActivatableClass(TheoraDecoderTransform);
#else
//CoCreatableClass(TheoraDecoderTransform);
#endif

const DecoderTransformRegisterInfo TheoraDecoderTransform::RegisterInfos[1] =
{
	{ MFVideoFormat_Theora, MFVideoFormat_YV12 },
};

TheoraDecoderTransform::TheoraDecoderTransform()
{
}

void TheoraDecoderTransform::OnValidateInputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Video) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find_if(std::begin(RegisterInfos), std::end(RegisterInfos), [&](auto&& info)
	{
		return info.InputSubType == subType;
	}) == std::end(RegisterInfos))
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

void TheoraDecoderTransform::OnValidateOutputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Video) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find_if(availableOutputTypes.begin(), availableOutputTypes.end(), [&](auto&& myType)
	{
		GUID mySubType;
		ThrowIfFailed(myType->GetGUID(MF_MT_SUBTYPE, &mySubType));
		return mySubType == subType;
	}) == availableOutputTypes.end())
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

DWORD TheoraDecoderTransform::OnGetOutputFrameSize() const noexcept
{
	switch (theoraInfo.pixel_fmt)
	{
	case TH_PF_420:
		return theoraInfo.frame_width * theoraInfo.frame_height * 1.5;
		break;
	default:
		return 0;
	}
}

WRL::ComPtr<IMFMediaType> TheoraDecoderTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeAvailableOutputTypes(type);
	return type;
}

WRL::ComPtr<IMFMediaType> TheoraDecoderTransform::OnSetOutputType(IMFMediaType * type)
{
	return type;
}

namespace
{
	ogg_packet CreatePacket(IMFSample* sample, BYTE* data, DWORD length)
	{
		ogg_packet packet;
		UINT32 bos, eos;
		UINT64 packetno;
		ThrowIfFailed(sample->GetUINT32(MF_MT_OGG_PACKET_BOS, &bos));
		ThrowIfFailed(sample->GetUINT32(MF_MT_OGG_PACKET_EOS, &eos));
		ThrowIfFailed(sample->GetUINT64(MF_MT_OGG_PACKET_NO, &packetno));

		packet.bytes = long(length);
		packet.b_o_s = long(bos);
		packet.e_o_s = long(eos);
		ThrowIfFailed(sample->GetSampleTime(&packet.granulepos));
		packet.packet = data;
		packet.packetno = ogg_int64_t(packetno);
		return packet;
	}
}

bool TheoraDecoderTransform::OnReceiveInput(IMFSample * sample)
{
	ComPtr<IMFMediaBuffer> buffer;
	ThrowIfFailed(sample->ConvertToContiguousBuffer(buffer.GetAddressOf()));

	BYTE* data; DWORD length;
	Core::MFBufferLocker locker(buffer.Get());
	locker.Lock(data, nullptr, &length);
	auto packet = CreatePacket(sample, data, length);

	// Header
	if (theoraState == TheoraState::Header)
	{
		auto ret = th_decode_headerin(&theoraInfo, &theoraComment, &theoraSetup._Myptr(), &packet);
		// 遇到了 Body
		if (ret == 0)
		{
			theoraContext.reset(th_decode_alloc(&theoraInfo, theoraSetup.get()));
			if (!theoraContext) ThrowIfFailed(E_OUTOFMEMORY);
			theoraSetup.reset();

			theoraState = TheoraState::Body;
			return FeedBodyPacket(packet);
		}
		// 忽略错误
		return false;
	}
	// Body
	return FeedBodyPacket(packet);
}

void TheoraDecoderTransform::OnProduceOutput(MFT_OUTPUT_DATA_BUFFER & output)
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

		switch (theoraInfo.pixel_fmt)
		{
		case TH_PF_420:
			cntLength = FillYV12Frame(buffer.Get(), data, maxLength);
			break;
		default:
			ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
		}
	}
	ThrowIfFailed(buffer->SetCurrentLength(cntLength));
	memset(decodedBuffer, 0, sizeof(decodedBuffer));
}

WRL::ComPtr<IMFMediaType> TheoraDecoderTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}

void TheoraDecoderTransform::InitializeAvailableOutputTypes(IMFMediaType* inputType)
{
	availableOutputTypes.clear();

	ComPtr<IMFMediaType> outputMediaType;
	ThrowIfFailed(MFCreateMediaType(&outputMediaType));
	ThrowIfFailed(outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	UINT64 frameSize, frameRate, pixelAspect;
	if (SUCCEEDED(inputType->GetUINT64(MF_MT_FRAME_SIZE, &frameSize)))
		ThrowIfFailed(outputMediaType->SetUINT64(MF_MT_FRAME_SIZE, frameSize));
	if (SUCCEEDED(inputType->GetUINT64(MF_MT_FRAME_RATE, &frameRate)))
		ThrowIfFailed(outputMediaType->SetUINT64(MF_MT_FRAME_RATE, frameRate));
	if(SUCCEEDED(inputType->GetUINT64(MF_MT_PIXEL_ASPECT_RATIO, &pixelAspect)))
		ThrowIfFailed(outputMediaType->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, pixelAspect));
	if (MFGetAttributeUINT32(inputType, MF_MT_PAN_SCAN_ENABLED, FALSE))
	{
		ThrowIfFailed(outputMediaType->SetUINT32(MF_MT_PAN_SCAN_ENABLED, TRUE));
		UINT32 blobSize;
		ThrowIfFailed(inputType->GetBlobSize(MF_MT_PAN_SCAN_APERTURE, &blobSize));
		auto aspectBuffer = std::make_unique<UINT8[]>(blobSize);
		ThrowIfFailed(inputType->GetBlob(MF_MT_PAN_SCAN_APERTURE, aspectBuffer.get(), blobSize, &blobSize));
		ThrowIfFailed(outputMediaType->SetBlob(MF_MT_PAN_SCAN_APERTURE, aspectBuffer.get(), blobSize));
	}
	UINT32 pixelFormatU32;
	ThrowIfFailed(inputType->GetUINT32(MF_MT_THEORA_PIXEL_FORMAT, &pixelFormatU32));
	auto pixelFormat = static_cast<th_pixel_fmt>(pixelFormatU32);
	switch (pixelFormat)
	{
	case TH_PF_420:
		ThrowIfFailed(outputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YV12));
		break;
	default:
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	}

	availableOutputTypes.emplace_back(std::move(outputMediaType));
}

void TheoraDecoderTransform::InitializeDecoder()
{
	theoraInfo.reset();
	theoraComment.reset();
	theoraSetup.reset();
}

bool TheoraDecoderTransform::FeedBodyPacket(ogg_packet & packet)
{
	auto ret = th_decode_packetin(theoraContext.get(), &packet, &granpos);
	// 产出帧
	if (ret == 0)
	{
		ThrowIfNot(th_decode_ycbcr_out(theoraContext.get(), decodedBuffer) == 0, 
			L"Unexpected decoding error.");
		return true;
	}
	// 不支持的格式
	else if (ret == TH_EIMPL)
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 忽略其它错误
	return false;
}

DWORD TheoraDecoderTransform::FillYV12Frame(IMFMediaBuffer* buffer, BYTE * data, DWORD maxLength)
{
	auto cntData = data;
	// Y' -> Y
	size_t planeSize = decodedBuffer[0].width * decodedBuffer[0].height;
	if (maxLength < planeSize) ThrowIfFailed(E_INVALIDARG);
	ThrowIfFailed(MFCopyImage(cntData, decodedBuffer[0].width, decodedBuffer[0].data, 
		decodedBuffer[0].stride, decodedBuffer[0].width, decodedBuffer[0].height));
	cntData += planeSize;
	maxLength -= planeSize;
	// Cr' -> V
	planeSize = decodedBuffer[2].width * decodedBuffer[2].height;
	if (maxLength < planeSize) ThrowIfFailed(E_INVALIDARG);
	ThrowIfFailed(MFCopyImage(cntData, decodedBuffer[2].width, decodedBuffer[2].data,
		decodedBuffer[2].stride, decodedBuffer[2].width, decodedBuffer[2].height));
	cntData += planeSize;
	maxLength -= planeSize;
	// Cb' -> U
	planeSize = decodedBuffer[1].width * decodedBuffer[1].height;
	if (maxLength < planeSize) ThrowIfFailed(E_INVALIDARG);
	ThrowIfFailed(MFCopyImage(cntData, decodedBuffer[1].width, decodedBuffer[1].data,
		decodedBuffer[1].stride, decodedBuffer[1].width, decodedBuffer[1].height));
	cntData += planeSize;
	maxLength -= planeSize;

	return DWORD(cntData - data);
}
