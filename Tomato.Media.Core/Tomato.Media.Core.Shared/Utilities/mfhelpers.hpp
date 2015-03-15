//
// Tomato Media
// Media Foundation 辅助
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../include/tomato.media.core.h"
#include <mfidl.h>

namespace Tomato
{
	class mfbuffer_locker
	{
	public:
		mfbuffer_locker(IMFMediaBuffer* buffer)
			:buffer(buffer)
		{

		}

		~mfbuffer_locker()
		{
			if (buffer)
				buffer->Unlock();
		}

		HRESULT lock(BYTE** data, DWORD* maxLength, DWORD* currentLength)
		{
			return buffer->Lock(data, maxLength, currentLength);
		}
	private:
		IMFMediaBuffer* buffer;
	};
}