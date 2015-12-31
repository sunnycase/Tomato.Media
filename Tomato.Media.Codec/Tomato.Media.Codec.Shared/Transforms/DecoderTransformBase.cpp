//
// Tomato Media Codec
// Media Foundation 解码 Transform 基类
// 
// 作者：SunnyCase
// 创建时间：2015-09-08
#include "pch.h"
#include "DecoderTransformBase.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

DecoderTransformBase::DecoderTransformBase()
{

}

DecoderTransformBase::~DecoderTransformBase()
{

}

#if (NTDDI_VERSION >= NTDDI_WIN8)
// IMediaExtension methods

//-------------------------------------------------------------------
// Name: SetProperties
// Sets the configuration of the decoder
//-------------------------------------------------------------------
IFACEMETHODIMP DecoderTransformBase::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
	return S_OK;
}

#endif

// IMFTransform methods. Refer to the Media Foundation SDK documentation for details.

//-------------------------------------------------------------------
// Name: GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::GetStreamLimits(
	DWORD   *pdwInputMinimum,
	DWORD   *pdwInputMaximum,
	DWORD   *pdwOutputMinimum,
	DWORD   *pdwOutputMaximum
	)
{
	ARGUMENT_NOTNULL_HR(pdwInputMinimum && pdwInputMaximum && pdwOutputMinimum && pdwOutputMaximum);

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

HRESULT DecoderTransformBase::GetStreamCount(
	DWORD   *pcInputStreams,
	DWORD   *pcOutputStreams
	)
{
	ARGUMENT_NOTNULL_HR(pcInputStreams && pcOutputStreams);

	// This MFT has a fixed number of streams.
	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return S_OK;
}

//-------------------------------------------------------------------
// Name: GetStreamIDs
// Returns stream IDs for the input and output streams.
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::GetStreamIDs(
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

HRESULT DecoderTransformBase::GetInputStreamInfo(
	DWORD                     dwInputStreamID,
	MFT_INPUT_STREAM_INFO *   pStreamInfo
	)
{
	ARGUMENT_NOTNULL_HR(pStreamInfo);

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
bool DecoderTransformBase::IsValidInputStream(DWORD inputStreamId) const noexcept
{
	// 只有一个输入流
	return inputStreamId == 0;
}

// IsValidOutputStream: Returns TRUE if dwOutputStreamID is a valid output stream identifier.
bool DecoderTransformBase::IsValidOutputStream(DWORD outputStreamId) const noexcept
{
	// 只有一个输出流
	return outputStreamId == 0;
}

DWORD DecoderTransformBase::OnGetOutputStreamFlags() const noexcept
{
	return MFT_OUTPUT_STREAM_WHOLE_SAMPLES;
}
//-------------------------------------------------------------------
// Name: GetOutputStreamInfo
// Returns information about an output stream.
//-------------------------------------------------------------------
// 备注：当没有 MediaType 的时候也应返回有效信息
HRESULT DecoderTransformBase::GetOutputStreamInfo(
	DWORD                     dwOutputStreamID,
	MFT_OUTPUT_STREAM_INFO *  pStreamInfo
	)
{
	ARGUMENT_NOTNULL_HR(pStreamInfo);

	if (!IsValidOutputStream(dwOutputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// NOTE: This method should succeed even when there is no media type on the
	//       stream. If there is no media type, we only need to fill in the dwFlags
	//       member of MFT_OUTPUT_STREAM_INFO. The other members depend on having a
	//       a valid media type.
	pStreamInfo->dwFlags = OnGetOutputStreamFlags();

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

HRESULT DecoderTransformBase::GetAttributes(IMFAttributes** /*pAttributes*/)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetInputStreamAttributes
// Returns stream-level attributes for an input stream.
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::GetInputStreamAttributes(
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

HRESULT DecoderTransformBase::GetOutputStreamAttributes(
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

HRESULT DecoderTransformBase::DeleteInputStream(DWORD /*dwStreamID*/)
{
	// This MFT has a fixed number of input streams, so the method is not implemented.
	return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: AddInputStreams
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::AddInputStreams(
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

HRESULT DecoderTransformBase::GetInputAvailableType(
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

HRESULT DecoderTransformBase::GetOutputAvailableType(
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

HRESULT DecoderTransformBase::SetInputType(
	DWORD           dwInputStreamID,
	IMFMediaType    *pType, // Can be NULL to clear the input type.
	DWORD           dwFlags
	)
{
	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	std::lock_guard<decltype(stateMutex)> locker(stateMutex);
	// 如果有输出则不能更改类型
	if (state == PendingOutput)
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

HRESULT DecoderTransformBase::SetOutputType(
	DWORD           dwOutputStreamID,
	IMFMediaType    *pType, // Can be NULL to clear the output type.
	DWORD           dwFlags
	)
{
	if (!IsValidOutputStream(dwOutputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	std::lock_guard<decltype(stateMutex)> locker(stateMutex);
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

void DecoderTransformBase::ValidateInputType(IMFMediaType *pmt)
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

void DecoderTransformBase::ValidateOutputType(IMFMediaType *pmt)
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
		ThrowIfFailed(MF_E_TRANSFORM_TYPE_NOT_SET);

	OnValidateOutputType(pmt);
}

//-------------------------------------------------------------------
// Name: GetInputCurrentType
// Description: Returns the current input type.
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::GetInputCurrentType(
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

HRESULT DecoderTransformBase::GetOutputCurrentType(
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

HRESULT DecoderTransformBase::GetInputStatus(
	DWORD           dwInputStreamID,
	DWORD           *pdwFlags
	)
{
	if (!pdwFlags) return E_POINTER;

	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	std::lock_guard<decltype(stateMutex)> locker(stateMutex);
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

HRESULT DecoderTransformBase::GetOutputStatus(DWORD *pdwFlags)
{
	if (!pdwFlags) return E_POINTER;

	// We can produce an output sample if (and only if)
	// we have an input sample.

	std::lock_guard<decltype(stateMutex)> locker(stateMutex);
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

HRESULT DecoderTransformBase::SetOutputBounds(
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

HRESULT DecoderTransformBase::ProcessEvent(
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

HRESULT DecoderTransformBase::ProcessMessage(
	MFT_MESSAGE_TYPE    eMessage,
	ULONG_PTR           /*ulParam*/
	)
{
	try
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
	CATCH_ALL();
}

//-------------------------------------------------------------------
// Name: ProcessInput
// Description: Process an input sample.
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::ProcessInput(
	DWORD               dwInputStreamID,
	IMFSample           *pSample,
	DWORD               dwFlags
	)
{
	ComPtr<DecoderTransformBase> thisGuard(this);
	if (!pSample) return E_INVALIDARG;

	if (!IsValidInputStream(dwInputStreamID))
		return MF_E_INVALIDSTREAMNUMBER;

	// dwFlags is reserved and must be zero.
	if (dwFlags != 0)
		return E_INVALIDARG;

	std::lock_guard<decltype(stateMutex)> locker(stateMutex);
	// Client must set input and output types.
	if (inputMediaType == nullptr || outputMediaType == nullptr)
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	if (state != TransformState::WaitingInput)
		return MF_E_NOTACCEPTING;

	try
	{
		if (OnReceiveInput(pSample))
			state = TransformState::PendingOutput;
	}
	CATCH_ALL();
	return S_OK;
}

//-------------------------------------------------------------------
// Name: ProcessOutput
// Description: Process an output sample.
//-------------------------------------------------------------------

HRESULT DecoderTransformBase::ProcessOutput(
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

	ComPtr<DecoderTransformBase> thisGuard(this);
	std::lock_guard<decltype(stateMutex)> locker(stateMutex);
	// If we don't have an input sample, we need some input before
	// we can generate any output.
	if (state != TransformState::PendingOutput)
		return MF_E_TRANSFORM_NEED_MORE_INPUT;
	try
	{
		OnProduceOutput(pOutputSamples[0]);

		// 如果已输出完毕则更改状态
		if (!(pOutputSamples[0].dwStatus & MFT_OUTPUT_DATA_BUFFER_INCOMPLETE))
			state = TransformState::WaitingInput;
	}
	CATCH_ALL();
	return S_OK;
}

void DecoderTransformBase::BeginStreaming()
{

}

void DecoderTransformBase::EndStreaming()
{

}