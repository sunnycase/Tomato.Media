//
// Tomato Media
// LibAV Media Foundation Transform
// 
// (c) SunnyCase 
// 创建日期 2015-03-16
#include "pch.h"
#include "LibAVMFTransform.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

struct avcodec_registry
{
	avcodec_registry()
	{
		avcodec_register_all();
	}
};

ActivatableClass(LibAVMFTransform);

LibAVMFTransform::LibAVMFTransform()
{
	static avcodec_registry avReg;

}

void LibAVMFTransform::OnValidateInputType(IMFMediaType * type)
{
}

void LibAVMFTransform::OnValidateOutputType(IMFMediaType * type)
{
}

DWORD LibAVMFTransform::OnGetOutputFrameSize() const noexcept
{
	return 0;
}

ComPtr<IMFMediaType> LibAVMFTransform::OnSetInputType(IMFMediaType * type)
{
	return ComPtr<IMFMediaType>();
}

ComPtr<IMFMediaType> LibAVMFTransform::OnSetOutputType(IMFMediaType * type)
{
	return ComPtr<IMFMediaType>();
}

void LibAVMFTransform::OnReceiveInput(IMFSample * sample)
{
}

void LibAVMFTransform::OnProduceOutput(IMFSample * input, MFT_OUTPUT_DATA_BUFFER & output)
{
}

ComPtr<IMFMediaType> LibAVMFTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	return ComPtr<IMFMediaType>();
}
