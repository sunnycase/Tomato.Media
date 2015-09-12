//
// Tomato Media Codec
// Media Foundation MediaSource 基类
// 
// 作者：SunnyCase
// 创建时间：2015-03-17
#pragma once
#include "common.h"
#include "../../include/MFOperationQueue.h"
#include "MediaSourceOperation.h"
#include "../../include/WeakReferenceBase.h"
#include <mfidl.h>
#include <ppltasks.h>
#include <atomic>

DEFINE_NS_MEDIA_CODEC

enum class MFMediaSourceState
{
	NotInitialized,
	Initializing,
	Starting,
	Started,
	Paused,
	Stopping,
	Stopped
};

class MediaSourceBase : public Core::WeakReferenceBase<MediaSourceBase,
	WRL::RuntimeClassFlags<WRL::ClassicCom>,
	IMFMediaEventGenerator,
	IMFMediaSource,
	IMFGetService>
{
public:
	using TOperation = std::shared_ptr<MediaSourceOperation>;

	virtual ~MediaSourceBase();

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback *pCallback, IUnknown *punkState) final;
	STDMETHODIMP EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
	STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent);
	STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue);

	// IMFMediaSource
	STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor **ppPresentationDescriptor);
	STDMETHODIMP GetCharacteristics(DWORD *pdwCharacteristics);
	STDMETHODIMP Pause();
	STDMETHODIMP Shutdown();
	STDMETHODIMP Start(
		IMFPresentationDescriptor *pPresentationDescriptor,
		const GUID *pguidTimeFormat,
		const PROPVARIANT *pvarStartPosition
		);
	STDMETHODIMP Stop();

	// IMFGetService
	IFACEMETHOD(GetService) (_In_ REFGUID guidService, _In_ REFIID riid, _Out_opt_ LPVOID *ppvObject);

	// 打开字节流
	concurrency::task<void> OpenAsync(IMFByteStream* byteStream);
	// 将操作入队
	void QueueAsyncOperation(TOperation&& operation);
	void QueueAsyncOperation(MediaSourceOperationKind operation);
protected:
	MediaSourceBase();

	HRESULT QueueEventUnk(MediaEventType met, REFGUID guidExtendedType,
		HRESULT hrStatus, IUnknown *unk);

	// 获取音源特性
	virtual DWORD OnGetCharacteristics() = 0;
	// 关闭
	virtual void OnShutdown();
	// 验证 Presentation Descriptor 有效
	virtual void OnValidatePresentationDescriptor(IMFPresentationDescriptor *pPD) = 0;
	// 验证开始操作
	virtual void ValidateStartOperation(MFMediaSourceState state, MFTIME position);
	// 创建 Presentation Descriptor
	virtual concurrency::task<WRL::ComPtr<IMFPresentationDescriptor>>
		OnCreatePresentationDescriptor(IMFByteStream* stream) = 0;
	// 流请求数据
	virtual concurrency::task<void> OnStreamsRequestData(IMFMediaStream* mediaStream) = 0;
	virtual void OnSeekSource(MFTIME position) = 0;
	// 开始流
	virtual void OnStartStream(DWORD streamId, bool selected, const PROPVARIANT& position) = 0;
	virtual void OnPauseStream(DWORD streamId) = 0;
	virtual void OnStopStream(DWORD streamId) = 0;
private:
	bool HasShutdown() const;
	// 验证 Presentation Descriptor 有效
	void ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);
	// 创建 Presentation Descriptor
	concurrency::task<void> CreatePresentationDescriptor(IMFByteStream* stream);
	void OnDispatchOperation(TOperation& op);
	// 加载音源
	concurrency::task<void> InitializeAudioSource(IMFByteStream* stream);

	// 开始
	void DoStart(MediaSourceStartOperation* operation);
	void StartStreams(IMFPresentationDescriptor* pd, const PROPVARIANT& position);

	void DoPause();
	void PauseStreams();

	void DoStop();
	void StopStreams();

	// 流停止
	void OnEndOfStream();
protected:
	MFMediaSourceState state = MFMediaSourceState::NotInitialized;	// 状态
	WRL::ComPtr<IMFMediaEventQueue> eventQueue;					// 事件队列
	std::mutex stateMutex;
private:
	std::shared_ptr<Core::MFOperationQueue<TOperation>> operationQueue;
	WRL::ComPtr<IMFPresentationDescriptor> presentDescriptor;	// PresentationDescriptor
	std::atomic<uint32_t> pendingEOSCount = 0;					// 需要结束的流数量
};

END_NS_MEDIA_CODEC