//
// Tomato Media Core
// Wrappers
// 作者：SunnyCase
// 创建时间：2016-01-15
//
#pragma once
#include "common.h"
#include <windows.storage.streams.h>
#include <mfidl.h>

DEFINE_NS_CORE

void TOMATO_CORE_API CreateBufferOnMFMediaBuffer(IMFMediaBuffer* sourceBuffer, ABI::Windows::Storage::Streams::IBuffer** wrappedBuffer);

END_NS_CORE