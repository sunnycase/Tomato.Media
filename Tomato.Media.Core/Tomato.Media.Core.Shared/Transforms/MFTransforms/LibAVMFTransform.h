//
// Tomato Media
// [Internal] LibAV Media Foundation Transform
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "AudioFrameDecoderBase.h"

NSDEF_TOMATO_MEDIA

class LibAVMFTransform : public AudioFrameDecoderBase
{
	InspectableClass(L"Tomato.Media.LibAVMFTransform", BaseTrust)
public:
	LibAVMFTransform();
private:
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
};

NSED_TOMATO_MEDIA