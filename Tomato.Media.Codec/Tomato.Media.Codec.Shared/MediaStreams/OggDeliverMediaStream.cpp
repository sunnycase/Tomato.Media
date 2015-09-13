//
// Tomato Media Codec
// 负责发送数据的 Ogg Media Stream
// 
// 作者：SunnyCase
// 创建时间：2015-09-09
#include "pch.h"
#include "OggDeliverMediaStream.h"
#include "constants.h"
#include "../../include/MFRAII.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

#define LOCK_STREAM() std::lock_guard<decltype(streamStateMutex)> locker(streamStateMutex)

OggDeliverMediaStream::OggDeliverMediaStream(ogg_stream_state && streamState, NS_CORE::WeakRef<MediaSourceBase> mediaSource, IMFStreamDescriptor * streamDescriptor)
	:DeliverMediaStreamBase(std::move(mediaSource), streamDescriptor), streamState(std::move(streamState))
{
}

void OggDeliverMediaStream::DeliverOggPage(ogg_page & page)
{
	LOCK_STREAM();
	if (ogg_stream_pagein(&streamState, &page) != 0)
	{
		// 忽略错误
		OutputDebugString(L"ogg page in stream occurred error.");
	}
	QueuePackets();
}

void OggDeliverMediaStream::QueuePackets()
{
	ogg_packet packet;
	while (ogg_stream_packetout(&streamState, &packet) == 1)
	{
		if (packet.bytes)
		{
			ComPtr<IMFSample> sample;
			ThrowIfFailed(MFCreateSample(sample.GetAddressOf()));
			ThrowIfFailed(sample->SetSampleTime(packet.granulepos));
			ThrowIfFailed(sample->SetUINT64(MF_MT_OGG_PACKET_NO, UINT64(packet.packetno)));
			ThrowIfFailed(sample->SetUINT32(MF_MT_OGG_PACKET_BOS, UINT32(packet.b_o_s)));
			ThrowIfFailed(sample->SetUINT32(MF_MT_OGG_PACKET_EOS, UINT32(packet.e_o_s)));

			ComPtr<IMFMediaBuffer> buffer;
			ThrowIfFailed(MFCreateAlignedMemoryBuffer(DWORD(packet.bytes), 64, buffer.GetAddressOf()));
			{
				Core::MFBufferLocker locker(buffer.Get());
				BYTE* data; DWORD maxLength;
				locker.Lock(data, &maxLength, nullptr);
				if (maxLength < DWORD(packet.bytes)) ThrowIfFailed(E_OUTOFMEMORY);
				ThrowIfNot(memcpy_s(data, maxLength, packet.packet, size_t(packet.bytes)) == 0,
					L"Cannot copy packet.");
			}
			ThrowIfFailed(buffer->SetCurrentLength(DWORD(packet.bytes)));
			ThrowIfFailed(sample->AddBuffer(buffer.Get()));
			EnqueueSample(sample.Get());
		}
		else
			RequestData();
		if (packet.e_o_s)
			EndOfDeliver();
	}
}
