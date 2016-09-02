//
// Tomato Media Codec
// Media Foundation 解码 Transform 基类
// 
// 作者：SunnyCase
// 创建时间：2015-09-08
#pragma once
#include <Tomato.Media/Tomato.Media.h>
#include <mutex>
#include <windows.media.h>
#include <mfidl.h>

DEFINE_NS_MEDIA_CODEC

struct DecoderTransformRegisterInfo
{
	const GUID InputSubType;
	const GUID OutputSubType;
};

// Media Foundation 解码 Transform 基类
class __declspec(novtable) DecoderTransformBase : public WRL::RuntimeClass<WRL::RuntimeClassFlags<
#if (NTDDI_VERSION >= NTDDI_WIN8)
	WRL::WinRtClassicComMix>,
	ABI::Windows::Media::IMediaExtension,
#else
	WRL::ClassicCom>,
#endif
	IMFTransform>
{
public:
	enum TransformState
	{
		Initializing,
		Ready,
		WaitingInput,
		PendingOutput
	};

	DecoderTransformBase();
	virtual ~DecoderTransformBase();

#if (NTDDI_VERSION >= NTDDI_WIN8)
	// IMediaExtension
	IFACEMETHOD(SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration) override;
#endif
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
	virtual DWORD OnGetOutputStreamFlags() const noexcept;
	// 获取输出帧大小
	virtual DWORD OnGetOutputFrameSize() const noexcept = 0;
	// 设置输入类型
	virtual WRL::ComPtr<IMFMediaType> OnSetInputType(IMFMediaType* type) = 0;
	// 设置输出类型
	virtual WRL::ComPtr<IMFMediaType> OnSetOutputType(IMFMediaType* type) = 0;
	// 接收输入
	// 返回值：是否可以输出采样
	virtual bool OnReceiveInput(IMFSample* sample) = 0;
	virtual void OnProduceOutput(MFT_OUTPUT_DATA_BUFFER& output) = 0;
	virtual WRL::ComPtr<IMFMediaType> OnGetOutputAvailableType(DWORD index) noexcept = 0;
	// 开始流水
	virtual void BeginStreaming();
	// 停止流水
	virtual void EndStreaming();
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
	TransformState state = Ready;						// 状态
	std::mutex stateMutex;
	WRL::ComPtr<IMFMediaType> inputMediaType;	// 输入媒体类型
	WRL::ComPtr<IMFMediaType> outputMediaType;	// 输出媒体类型
private:
};

END_NS_MEDIA_CODEC