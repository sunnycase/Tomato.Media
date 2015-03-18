//
// Tomato Media
// Media Foundation 单流 Audio Source 基类
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#include "pch.h"
#include "SingleStreamAudioSourceBase.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

SingleStreamAudioSourceBase::SingleStreamAudioSourceBase()
{

}

SingleStreamAudioSourceBase::~SingleStreamAudioSourceBase()
{

}

// 验证是否只有 1 个流
void SingleStreamAudioSourceBase::OnValidatePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
	DWORD stream_count;

	THROW_IF_FAILED(pPD->GetStreamDescriptorCount(&stream_count));
	if (stream_count != 1)
		THROW_IF_FAILED(MF_E_INVALIDSTREAMNUMBER);
}

task<ComPtr<IMFPresentationDescriptor>>
SingleStreamAudioSourceBase::OnCreatePresentationDescriptor(ComPtr<IMFByteStream> stream)
{
	return InitializeAudioStream(stream).then([this]
	{
		ComPtr<IMFPresentationDescriptor> pd;
		ComPtr<IMFStreamDescriptor> streamDesc;

		THROW_IF_FAILED(audioStream->GetStreamDescriptor(&streamDesc));
		THROW_IF_FAILED(MFCreatePresentationDescriptor(1, streamDesc.GetAddressOf(), &pd));
		THROW_IF_FAILED(pd->SelectStream(0));
		OnConfigurePresentationDescriptor(pd.Get());

		return pd;
	});
}

void SingleStreamAudioSourceBase::OnConfigurePresentationDescriptor(IMFPresentationDescriptor *pPD)
{

}

task<void> SingleStreamAudioSourceBase::InitializeAudioStream(ComPtr<IMFByteStream> stream)
{
	return OnCreateMediaTypes(stream).then([this](std::vector<ComPtr<IMFMediaType>> mediaTypes)
	{
		ComPtr<IMFStreamDescriptor> streamDesc;
		ComPtr<IMFMediaTypeHandler> typeHandler;

		if (mediaTypes.size() == 0) THROW_IF_FAILED(MF_E_INVALIDMEDIATYPE);

		// 创建 stream descriptor
		THROW_IF_FAILED(MFCreateStreamDescriptor(0, mediaTypes.size(), 
			mediaTypes[0].GetAddressOf(), &streamDesc));
		THROW_IF_FAILED(streamDesc->GetMediaTypeHandler(&typeHandler));
		// 设定当前 Media Type
		THROW_IF_FAILED(typeHandler->SetCurrentMediaType(mediaTypes[0].Get()));
		// 创建Stream
		audioStream = Make<MFAudioStream>(this, streamDesc);
	});
}

void SingleStreamAudioSourceBase::OnStartStream(DWORD streamId, bool selected, REFERENCE_TIME position)
{
	if (streamId != 0) THROW_IF_FAILED(E_INVALIDARG);

	OnStartAudioStream(position);
	// 判断流是不是已经启动了
	auto started = audioStream->GetState() == MFMediaStreamState::Started;
	if (selected)
		audioStream->Start(position);
	else if (started)
		audioStream->Pause();

	auto met = started ? MEUpdatedStream : MENewStream;

	THROW_IF_FAILED(QueueEventUnk(met, GUID_NULL, S_OK, reinterpret_cast<IUnknown*>(audioStream.Get())));
}

task<void> SingleStreamAudioSourceBase::OnStreamsRequestData(TOperation& op)
{
	ComPtr<IMFSample> sample;
	THROW_IF_FAILED(MFCreateSample(&sample));

	return OnReadSample(sample).then([this, sample](bool read)
	{
		if (read)
			audioStream->DeliverPayload(sample.Get());
		else
			audioStream->DeliverPayload(nullptr);
	});
}

void SingleStreamAudioSourceBase::EndOfDeliver()
{
	audioStream->EndOfDeliver();
}