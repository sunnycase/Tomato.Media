#include "pch.h"
#include "common.h"
#include "minizip/aes/entropy.h"

using namespace WRL;
using namespace WRL::Wrappers;
using namespace Windows::Foundation;
using namespace ABI::Windows::Security::Cryptography;
using namespace ABI::Windows::Storage::Streams;

extern "C" int entropy_fun(unsigned char buf[], unsigned int len)
{
	try
	{
		ComPtr<ICryptographicBufferStatics> crypBufferStatics;
		ThrowIfFailed(GetActivationFactory(HStringReference(RuntimeClass_Windows_Security_Cryptography_CryptographicBuffer).Get(), &crypBufferStatics));
		ComPtr<IBuffer> buffer;
		ThrowIfFailed(crypBufferStatics->GenerateRandom(len, &buffer));
		UINT32 outLength;
		BYTE* rnd;
		ThrowIfFailed(crypBufferStatics->CopyToByteArray(buffer.Get(), &outLength, &rnd));
		return memcpy_s(buf, len, rnd, outLength);
	}
	CATCH_ALL();
}