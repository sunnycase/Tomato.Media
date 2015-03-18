//
// Tomato Media
// [Internal] LibAV Media Foundation Transform
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "AudioFrameDecoderBase.h"
#include "Utilities/libavhelpers.h"

NSDEF_TOMATO_MEDIA

class LibAVMFTransform : public AudioFrameDecoderBase
{
	InspectableClass(L"Tomato.Media.LibAVMFTransform", BaseTrust)
public:
	LibAVMFTransform();
	virtual ~LibAVMFTransform();
private:
	STDMETHODIMP GetOutputStreamInfo(
		DWORD                     dwOutputStreamID,
		MFT_OUTPUT_STREAM_INFO *  pStreamInfo
		);

	virtual void OnValidateInputType(IMFMediaType* type);
	virtual void OnValidateOutputType(IMFMediaType* type);
	// 获取输出帧大小
	virtual DWORD OnGetOutputFrameSize() const noexcept;
	// 设置输入类型
	virtual wrl::ComPtr<IMFMediaType> OnSetInputType(IMFMediaType* type);
	// 设置输出类型
	virtual wrl::ComPtr<IMFMediaType> OnSetOutputType(IMFMediaType* type);
	// 接收输入
	virtual void OnReceiveInput(IMFSample* sample);
	virtual void OnProduceOutput(IMFSample* input, MFT_OUTPUT_DATA_BUFFER& output);
	virtual wrl::ComPtr<IMFMediaType> OnGetOutputAvailableType(DWORD index) noexcept;
private:
	void InitializeLibAVFormat(IMFMediaType* type);
	void InitializeAvailableOutputTypes();
	std::pair<uint32_t, bool> DecodeFrame(AVPacket& packet, wrl::ComPtr<IMFMediaBuffer>& buffer);
private:
	WAVEFORMATLIBAV waveFormat;
	WAVEFORMATEX outputFormat;
	std::vector<wrl::ComPtr<IMFMediaType>> availableOutputTypes;
};

NSED_TOMATO_MEDIA