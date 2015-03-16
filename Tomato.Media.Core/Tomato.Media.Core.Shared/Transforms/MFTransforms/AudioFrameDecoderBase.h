//
// Tomato Media
// [Internal] Media Foundation 音频帧解码器
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../../../include/tomato.media.core.h"
#include "MFTransform.h"

NSDEF_TOMATO_MEDIA

class AudioFrameDecoderBase : public wrl::RuntimeClass<wrl::RuntimeClassFlags<
	wrl::RuntimeClassType::WinRtClassicComMix>, ABI::Windows::Media::IMediaExtension, IMFTransform >
{
public:
	AudioFrameDecoderBase();
	virtual ~AudioFrameDecoderBase();

	// IMediaExtension
	IFACEMETHOD(SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration);

	// IMFTransform methods
	STDMETHODIMP GetStreamLimits(
		DWORD   *pdwInputMinimum,
		DWORD   *pdwInputMaximum,
		DWORD   *pdwOutputMinimum,
		DWORD   *pdwOutputMaximum
		);

	STDMETHODIMP GetStreamCount(
		DWORD   *pcInputStreams,
		DWORD   *pcOutputStreams
		);

	STDMETHODIMP GetStreamIDs(
		DWORD   dwInputIDArraySize,
		DWORD   *pdwInputIDs,
		DWORD   dwOutputIDArraySize,
		DWORD   *pdwOutputIDs
		);

	STDMETHODIMP GetInputStreamInfo(
		DWORD                     dwInputStreamID,
		MFT_INPUT_STREAM_INFO *   pStreamInfo
		);

	STDMETHODIMP GetOutputStreamInfo(
		DWORD                     dwOutputStreamID,
		MFT_OUTPUT_STREAM_INFO *  pStreamInfo
		);

	STDMETHODIMP GetAttributes(IMFAttributes** pAttributes);

	STDMETHODIMP GetInputStreamAttributes(
		DWORD           dwInputStreamID,
		IMFAttributes   **ppAttributes
		);

	STDMETHODIMP GetOutputStreamAttributes(
		DWORD           dwOutputStreamID,
		IMFAttributes   **ppAttributes
		);

	STDMETHODIMP DeleteInputStream(DWORD dwStreamID);

	STDMETHODIMP AddInputStreams(
		DWORD   cStreams,
		DWORD   *adwStreamIDs
		);

	STDMETHODIMP GetInputAvailableType(
		DWORD           dwInputStreamID,
		DWORD           dwTypeIndex, // 0-based
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetOutputAvailableType(
		DWORD           dwOutputStreamID,
		DWORD           dwTypeIndex, // 0-based
		IMFMediaType    **ppType
		);

	STDMETHODIMP SetInputType(
		DWORD           dwInputStreamID,
		IMFMediaType    *pType,
		DWORD           dwFlags
		);

	STDMETHODIMP SetOutputType(
		DWORD           dwOutputStreamID,
		IMFMediaType    *pType,
		DWORD           dwFlags
		);

	STDMETHODIMP GetInputCurrentType(
		DWORD           dwInputStreamID,
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetOutputCurrentType(
		DWORD           dwOutputStreamID,
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetInputStatus(
		DWORD           dwInputStreamID,
		DWORD           *pdwFlags
		);

	STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);

	STDMETHODIMP SetOutputBounds(
		LONGLONG        hnsLowerBound,
		LONGLONG        hnsUpperBound
		);

	STDMETHODIMP ProcessEvent(
		DWORD              dwInputStreamID,
		IMFMediaEvent      *pEvent
		);

	STDMETHODIMP ProcessMessage(
		MFT_MESSAGE_TYPE    eMessage,
		ULONG_PTR           ulParam
		);

	STDMETHODIMP ProcessInput(
		DWORD               dwInputStreamID,
		IMFSample           *pSample,
		DWORD               dwFlags
		);

	STDMETHODIMP ProcessOutput(
		DWORD                   dwFlags,
		DWORD                   cOutputBufferCount,
		MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
		DWORD                   *pdwStatus
		);
protected:
	// 如果未关闭则执行操作
	//HRESULT IfNotShutdown(std::function<HRESULT(MFTState)> aliveHandler) noexcept;

	// 验证输入类型
	virtual void OnValidateInputType(IMFMediaType* type) = 0;
	// 验证输出类型
	virtual void OnValidateOutputType(IMFMediaType* type) = 0;
	// 获取输出帧大小
	virtual DWORD OnGetOutputFrameSize() const noexcept = 0;
	// 设置输入类型
	virtual wrl::ComPtr<IMFMediaType> OnSetInputType(IMFMediaType* type) = 0;
	// 设置输出类型
	virtual wrl::ComPtr<IMFMediaType> OnSetOutputType(IMFMediaType* type) = 0;
	// 接收输入
	virtual void OnReceiveInput(IMFSample* sample) = 0;
	virtual void OnProduceOutput(IMFSample* input, MFT_OUTPUT_DATA_BUFFER& output) = 0;
	virtual wrl::ComPtr<IMFMediaType> OnGetOutputAvailableType(DWORD index) noexcept = 0;
	// 开始流水
	virtual void BeginStreaming() noexcept;
	// 停止流水
	virtual void EndStreaming() noexcept;
private:
	// 验证是否有效的输入流 Id
	bool IsValidInputStream(DWORD inputStreamId) const noexcept;
	// 验证是否有效的输出流 Id
	bool IsValidOutputStream(DWORD outputStreamId) const noexcept;
	// 验证输出类型
	void ValidateOutputType(IMFMediaType* type);
	// 验证输入类型
	void ValidateInputType(IMFMediaType* type);
protected:
	TransformState state;						// 状态
private:
	wrl::ComPtr<IMFSample> inputSample;			// 输入采样
	wrl::ComPtr<IMFMediaType> inputMediaType;	// 输入媒体类型
	wrl::ComPtr<IMFMediaType> outputMediaType;	// 输出媒体类型
};

NSED_TOMATO_MEDIA