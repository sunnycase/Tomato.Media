//
// Tomato Media Core
// Wrappers
// 作者：SunnyCase
// 创建时间：2016-01-15
//
#include "pch.h"
#include "Wrappers.h"
#include <robuffer.h>
#include "MFRAII.h"

using namespace NS_CORE;
using namespace WRL;

namespace
{
	class BufferOnMFMediaBuffer : public RuntimeClass<RuntimeClassFlags<WinRtClassicComMix>, ABI::Windows::Storage::Streams::IBuffer,
		Windows::Storage::Streams::IBufferByteAccess>
	{
	public:
		BufferOnMFMediaBuffer(IMFMediaBuffer * sourceBuffer)
			:_buffer(sourceBuffer), _bufferLock(sourceBuffer)
		{
			_bufferLock.Lock(_data, nullptr, nullptr);
		}

		STDMETHODIMP get_Capacity(UINT32 *value) override
		{
			DWORD length;
			auto hret = _buffer->GetMaxLength(&length);
			if(SUCCEEDED(hret))
				*value = length;
			return hret;
		}

		STDMETHODIMP get_Length(UINT32 *value) override
		{
			DWORD length;
			auto hret = _buffer->GetCurrentLength(&length);
			if (SUCCEEDED(hret))
				*value = length;
			return hret;
		}

		STDMETHODIMP put_Length(UINT32 value) override
		{
			return _buffer->SetCurrentLength(value);
		}

		STDMETHODIMP Buffer(byte **value) override
		{
			*value = _data;
			return S_OK;
		}
	private:
		ComPtr<IMFMediaBuffer> _buffer;
		MFBufferLocker _bufferLock;
		BYTE* _data;
	};
}

void NS_CORE::CreateBufferOnMFMediaBuffer(IMFMediaBuffer * sourceBuffer, ABI::Windows::Storage::Streams::IBuffer ** wrappedBuffer)
{
	if (!wrappedBuffer || !sourceBuffer) ThrowIfFailed(E_POINTER);
	auto wrapped = Make<BufferOnMFMediaBuffer>(sourceBuffer);
	*wrappedBuffer = wrapped.Detach();
}
