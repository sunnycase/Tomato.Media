//
// Tomato Media Codec
// 负责发送数据的 FFmpeg Media Stream
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#include "pch.h"
#include "FFmpegDeliverMediaStream.h"
#include "constants.h"
#include "../../include/MFRAII.h"
#include "Utilities/FFmpegHelper.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace NS_MEDIA_CODEC::FFmpeg;
using namespace WRL;
using namespace concurrency;

#define LOCK_STREAM() std::lock_guard<decltype(streamStateMutex)> locker(streamStateMutex)
#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> stateLocker(stateMutex)

FFmpegDeliverMediaStream::FFmpegDeliverMediaStream(AVStream* stream, NS_CORE::WeakRef<MediaSourceBase> mediaSource, IMFStreamDescriptor * streamDescriptor)
	:DeliverMediaStreamBase(std::move(mediaSource), streamDescriptor), _stream(stream)
{
}

void FFmpegDeliverMediaStream::DeliverPacket(AVPacketRAII&& packet)
{
	{
		LOCK_STATE();
		if (DeliverMediaStreamBase::streamState != StreamState::Started &&
			DeliverMediaStreamBase::streamState != StreamState::Paused)
			return;
	}
	LOCK_STREAM();
	QueuePacket(std::move(packet));
}

void FFmpegDeliverMediaStream::QueuePacket(AVPacketRAII&& packet)
{
	try
	{
		if (packet.buf)
		{
			ComPtr<IMFSample> sample;
			ThrowIfFailed(MFCreateSample(sample.GetAddressOf()));
			if (packet.pts != AV_NOPTS_VALUE)
				ThrowIfFailed(sample->SetSampleTime(dt2hns(packet.pts, _stream)));

			ComPtr<IMFMediaBuffer> mediaBuffer;
			CreateMFMediaBufferOnAVPacket(std::move(packet), &mediaBuffer);
			ThrowIfFailed(sample->AddBuffer(mediaBuffer.Get()));
			EnqueueSample(sample.Get());
		}
		else
			RequestData();
	}
	catch (...)
	{
		RequestData();
	}
}

void FFmpegDeliverMediaStream::OnResetStream()
{

}
