//
// Tomato Media
// Media Foundation 音频帧解码器
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "AudioFrameDecoderBase.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

AudioFrameDecoderBase::AudioFrameDecoderBase()
//:state_mach(MFTState::Setuping)
{

}

AudioFrameDecoderBase::~AudioFrameDecoderBase()
{

}

//HRESULT AudioFrameDecoderBase::IfNotShutdown(std::function<HRESULT(MFTState)> aliveHandler) noexcept
//{
//	try
//	{
//		mft_sm_t::locker_t locker(state_mach);
//		auto state = locker.get_state();
//
//		if (state == MFTState::NotInitialized)
//			return MF_E_SHUTDOWN;
//		else
//			return aliveHandler(state);
//	}
//	catch (COMException^ ex)
//	{
//		return ex->HResult;
//	}
//	catch (...)
//	{
//		return E_FAIL;
//	}
//}

// IMediaExtension methods

//-------------------------------------------------------------------
// Name: SetProperties
// Sets the configuration of the decoder
//-------------------------------------------------------------------
IFACEMETHODIMP AudioFrameDecoderBase::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
	return S_OK;
}

// IMFTransform methods. Refer to the Media Foundation SDK documentation for details.

//-------------------------------------------------------------------
// Name: GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetStreamLimits(
	DWORD   *pdwInputMinimum,
	DWORD   *pdwInputMaximum,
	DWORD   *pdwOutputMinimum,
	DWORD   *pdwOutputMaximum
	)
{

	if ((pdwInputMinimum == NULL) ||
		(pdwInputMaximum == NULL) ||
		(pdwOutputMinimum == NULL) ||
		(pdwOutputMaximum == NULL))
	{
		return E_POINTER;
	}


	// This MFT has a fixed number of streams.
	*pdwInputMinimum = 1;
	*pdwInputMaximum = 1;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetStreamCount
// Returns the actual number of streams.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetStreamCount(
	DWORD   *pcInputStreams,
	DWORD   *pcOutputStreams
	)
{
	if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))

	{
		return E_POINTER;
	}

	// This MFT has a fixed number of streams.
	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetStreamIDs
// Returns stream IDs for the input and output streams.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetStreamIDs(
	DWORD   /*dwInputIDArraySize*/,
	DWORD   * /*pdwInputIDs*/,
	DWORD   /*dwOutputIDArraySize*/,
	DWORD   * /*pdwOutputIDs*/
	)
{
	// Do not need to implement, because this MFT has a fixed number of
	// streams and the stream IDs match the stream indexes.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetInputStreamInfo
// Returns information about an input stream.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetInputStreamInfo(
	DWORD                     dwInputStreamID,
	MFT_INPUT_STREAM_INFO *   pStreamInfo
	)
{
	if (!pStreamInfo) return E_POINTER;

	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	pStreamInfo->hnsMaxLatency = 0;

	//  We can process data on any boundary.
	pStreamInfo->dwFlags = 0;

	pStreamInfo->cbSize = 1;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 1;

	return S_OK;
}

// IsValidOutputStream: Returns TRUE if dwOutputStreamID is a valid output stream identifier.
bool AudioFrameDecoderBase::IsValidInputStream(DWORD inputStreamId) const noexcept
{
	// 只有一个输入流
	return inputStreamId == 0;
}

// IsValidOutputStream: Returns TRUE if dwOutputStreamID is a valid output stream identifier.
bool AudioFrameDecoderBase::IsValidOutputStream(DWORD outputStreamId) const noexcept
{
	// 只有一个输出流
	return outputStreamId == 0;
}
//-------------------------------------------------------------------
// Name: GetOutputStreamInfo
// Returns information about an output stream.
//-------------------------------------------------------------------
// 备注：当没有 MediaType 的时候也应返回有效信息
HRESULT AudioFrameDecoderBase::GetOutputStreamInfo(
	DWORD                     dwOutputStreamID,
	MFT_OUTPUT_STREAM_INFO *  pStreamInfo
	)
{
	if (!pStreamInfo) return E_POINTER;

	if (!IsValidOutputStream(dwOutputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// NOTE: This method should succeed even when there is no media type on the
	//       stream. If there is no media type, we only need to fill in the dwFlags
	//       member of MFT_OUTPUT_STREAM_INFO. The other members depend on having a
	//       a valid media type.
	pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES |
		MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;

	if (!outputMediaType)
	{
		pStreamInfo->cbSize = 0;
		pStreamInfo->cbAlignment = 0;
	}
	else
	{
		pStreamInfo->cbSize = OnGetOutputFrameSize();
		pStreamInfo->cbAlignment = 16;
	}
	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetAttributes
// Returns the attributes for the MFT.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetAttributes(IMFAttributes** /*pAttributes*/)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetInputStreamAttributes
// Returns stream-level attributes for an input stream.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetInputStreamAttributes(
	DWORD           /*dwInputStreamID*/,
	IMFAttributes   ** /*ppAttributes*/
	)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetOutputStreamAttributes
// Returns stream-level attributes for an output stream.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetOutputStreamAttributes(
	DWORD           /*dwOutputStreamID*/,
	IMFAttributes   ** /*ppAttributes*/
	)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: DeleteInputStream
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::DeleteInputStream(DWORD /*dwStreamID*/)
{
	// This MFT has a fixed number of input streams, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: AddInputStreams
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::AddInputStreams(
	DWORD   /*cStreams*/,
	DWORD   * /*adwStreamIDs*/
	)
{
	// This MFT has a fixed number of output streams, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetInputAvailableType
// Description: Return a preferred input type.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetInputAvailableType(
	DWORD           /*dwInputStreamID*/,
	DWORD           /*dwTypeIndex*/,
	IMFMediaType    ** /*ppType*/
	)
{
	return MF_E_NO_MORE_TYPES;
}

//-------------------------------------------------------------------
// Name: GetOutputAvailableType
// Description: Return a preferred output type.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetOutputAvailableType(
	DWORD           dwOutputStreamID,
	DWORD           dwTypeIndex, // 0-based
	IMFMediaType    **ppType
	)
{
	if (!ppType)return E_INVALIDARG;

	if (!IsValidOutputStream(dwOutputStreamID)) return MF_E_INVALIDSTREAMNUMBER;

	if (!inputMediaType) return MF_E_TRANSFORM_TYPE_NOT_SET;

	auto outputType = OnGetOutputAvailableType(dwTypeIndex);
	if (outputType)
	{
		*ppType = outputType.Detach();
		return S_OK;
	}
	else
		return MF_E_NO_MORE_TYPES;
}

//-------------------------------------------------------------------
// Name: SetInputType
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::SetInputType(
	DWORD           dwInputStreamID,
	IMFMediaType    *pType, // Can be NULL to clear the input type.
	DWORD           dwFlags
	)
{
	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// 如果有输出则不能更改类型
	if (state == TransformState::PendingOutput)
		return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	try
	{
		if (pType)
			ValidateInputType(pType);
		// 设置类型
		if (!(dwFlags & MFT_SET_TYPE_TEST_ONLY))
			inputMediaType = OnSetInputType(pType);
	}
	CATCH_ALL();

	return S_OK;
}

//-------------------------------------------------------------------
// Name: SetOutputType
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::SetOutputType(
	DWORD           dwOutputStreamID,
	IMFMediaType    *pType, // Can be NULL to clear the output type.
	DWORD           dwFlags
	)
{
	if (!IsValidOutputStream(dwOutputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// 如果有输出则不能更改类型
	if (state == TransformState::PendingOutput)
		return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	try
	{
		if (pType)
			ValidateOutputType(pType);
		// 设置类型
		if (!(dwFlags & MFT_SET_TYPE_TEST_ONLY))
		{
			outputMediaType = OnSetOutputType(pType);
			state = TransformState::WaitingInput;
		}
	}
	CATCH_ALL();
	return S_OK;
}

void AudioFrameDecoderBase::ValidateInputType(IMFMediaType *pmt)
{
	//  Check if the type is already set and if so reject any type that's not identical.
	if (inputMediaType)
	{
		DWORD dwFlags = 0;
		if (S_OK == inputMediaType->IsEqual(pmt, &dwFlags))
			return;
	}

	OnValidateInputType(pmt);
}

void AudioFrameDecoderBase::ValidateOutputType(IMFMediaType *pmt)
{
	//  Check if the type is already set and if so reject any type that's not identical.
	if (outputMediaType)
	{
		DWORD dwFlags = 0;
		if (S_OK == outputMediaType->IsEqual(pmt, &dwFlags))
			return;
	}
	// Input type must be set first.
	if (!inputMediaType)
		THROW_IF_FAILED(MF_E_TRANSFORM_TYPE_NOT_SET);

	OnValidateOutputType(pmt);
}

//-------------------------------------------------------------------
// Name: GetInputCurrentType
// Description: Returns the current input type.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetInputCurrentType(
	DWORD           dwInputStreamID,
	IMFMediaType    **ppType
	)
{
	if (!ppType) return E_POINTER;

	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	if (!inputMediaType)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	inputMediaType.CopyTo(ppType);
	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetOutputCurrentType
// Description: Returns the current output type.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetOutputCurrentType(
	DWORD           dwInputStreamID,
	IMFMediaType    **ppType
	)
{
	if (!ppType) return E_POINTER;

	if (!IsValidOutputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	if (!outputMediaType)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	outputMediaType.CopyTo(ppType);
	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetInputStatus
// Description: Query if the MFT is accepting more input.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetInputStatus(
	DWORD           dwInputStreamID,
	DWORD           *pdwFlags
	)
{
	if (!pdwFlags) return E_POINTER;

	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// If we already have an input sample, we don't accept
	// another one until the client calls ProcessOutput or Flush.
	if (state == TransformState::WaitingInput)
		*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
	else
		*pdwFlags = 0;
	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetOutputStatus
// Description: Query if the MFT can produce output.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::GetOutputStatus(DWORD *pdwFlags)
{
	if (!pdwFlags) return E_POINTER;

	// We can produce an output sample if (and only if)
	// we have an input sample.
	if (state == TransformState::PendingOutput)
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	else
		*pdwFlags = 0;
	return S_OK;
}

//-------------------------------------------------------------------
// Name: SetOutputBounds
// Sets the range of time stamps that the MFT will output.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::SetOutputBounds(
	LONGLONG        /*hnsLowerBound*/,
	LONGLONG        /*hnsUpperBound*/
	)
{
	// Implementation of this method is optional.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: ProcessEvent
// Sends an event to an input stream.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::ProcessEvent(
	DWORD              /*dwInputStreamID*/,
	IMFMediaEvent      * /*pEvent */
	)
{
	// This MFT does not handle any stream events, so the method can
	// return E_NOTIMPL. This tells the pipeline that it can stop
	// sending any more events to this MFT.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: ProcessMessage
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::ProcessMessage(
	MFT_MESSAGE_TYPE    eMessage,
	ULONG_PTR           /*ulParam*/
	)
{
	switch (eMessage)
	{
	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		BeginStreaming();
		break;

	case MFT_MESSAGE_NOTIFY_END_STREAMING:
		EndStreaming();
		break;
	}
	return S_OK;
}

//-------------------------------------------------------------------
// Name: ProcessInput
// Description: Process an input sample.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::ProcessInput(
	DWORD               dwInputStreamID,
	IMFSample           *pSample,
	DWORD               dwFlags
	)
{
	if (!pSample) return E_INVALIDARG;

	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// dwFlags is reserved and must be zero.
	if (dwFlags != 0)
		return E_INVALIDARG;

	// Client must set input and output types.
	if (inputMediaType == nullptr || outputMediaType == nullptr)
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	if (state != TransformState::WaitingInput)
		return MF_E_NOTACCEPTING;

	try
	{
		std::lock_guard<decltype(stateMutex)> locker(stateMutex);

		OnReceiveInput(pSample);
		inputSample = pSample;
		state = TransformState::PendingOutput;
	}
	CATCH_ALL();
	return S_OK;
}

//-------------------------------------------------------------------
// Name: ProcessOutput
// Description: Process an output sample.
//-------------------------------------------------------------------

HRESULT AudioFrameDecoderBase::ProcessOutput(
	DWORD                   dwFlags,
	DWORD                   cOutputBufferCount,
	MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
	DWORD                   *pdwStatus
	)
{
	// Check input parameters...

	// There are no flags that we accept in this MFT.
	// The only defined flag is MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER. This
	// flag only applies when the MFT marks an output stream as lazy or optional.
	// However there are no lazy or optional streams on this MFT, so the flag is
	// not valid.
	if (dwFlags != 0) return E_INVALIDARG;

	if (pOutputSamples == nullptr || pdwStatus == nullptr)
		return E_POINTER;

	// Must be exactly one output buffer.
	if (cOutputBufferCount != 1)
		return E_INVALIDARG;

	// If we don't have an input sample, we need some input before
	// we can generate any output.
	if (state != TransformState::PendingOutput)
		return MF_E_TRANSFORM_NEED_MORE_INPUT;
	try
	{
		std::lock_guard<decltype(stateMutex)> locker(stateMutex);

		OnProduceOutput(inputSample.Get(), pOutputSamples[0]);

		// 如果已输出完毕则更改状态
		if (!(pOutputSamples[0].dwStatus & MFT_OUTPUT_DATA_BUFFER_INCOMPLETE))
		{
			inputSample.Reset();
			state = TransformState::WaitingInput;
		}
	}
	CATCH_ALL();
	return S_OK;
}

void AudioFrameDecoderBase::BeginStreaming() noexcept
{

}

void AudioFrameDecoderBase::EndStreaming() noexcept
{

}