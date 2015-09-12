//
// Tomato Media Codec
// 负责发送数据的 Ogg Media Stream
// 
// 作者：SunnyCase
// 创建时间：2015-09-09
#pragma once
#include "DeliverMediaStreamBase.h"
#include "libogg/ogg.h"

DEFINE_NS_MEDIA_CODEC

class OggDeliverMediaStream : public DeliverMediaStreamBase
{
public:
	OggDeliverMediaStream(ogg_stream_state && streamState, Core::WeakRef<MediaSourceBase> mediaSource, IMFStreamDescriptor* streamDescriptor);

	void DeliverOggPage(ogg_page& page);
	long GetSerialNo() const noexcept { return streamState.serialno; }
private:
	void QueuePackets();
private:
	ogg_stream_state streamState;
	std::mutex streamStateMutex;
};

END_NS_MEDIA_CODEC