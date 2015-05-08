//
// Tomato Media
// Media Foundation Audio Source 基类
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "../include/tomato.media.core.h"
#include "../MFOperationQueue.h"
#include "MediaSourceOperation.h"

NSDEF_TOMATO_MEDIA

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

class AudioSourceBase : public wrl::RuntimeClass<wrl::RuntimeClassFlags<
	wrl::RuntimeClassType::ClassicCom>,
	IMFMediaEventGenerator,
	IMFMediaSource,
	IMFGetService>, public MFOperationQueue<std::shared_ptr<MediaSourceOperation>>
{
public:
	using TOperation = MFOperationQueue::operation_t;

	virtual ~AudioSourceBase();

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
	AudioSourceBase();

	HRESULT QueueEventUnk(MediaEventType met, REFGUID guidExtendedType,
		HRESULT hrStatus, IUnknown *unk);

	// 获取音源特性
	virtual DWORD OnGetCharacteristics() = 0;
	// 关闭
	virtual void OnShutdown();
	// 验证 Presentation Descriptor 有效
	virtual void OnValidatePresentationDescriptor(IMFPresentationDescriptor *pPD) = 0;
	// 验证开始操作
	virtual void ValidateStartOperation(MFMediaSourceState state, REFERENCE_TIME position);
	// 创建 Presentation Descriptor
	virtual concurrency::task<wrl::ComPtr<IMFPresentationDescriptor>>
		OnCreatePresentationDescriptor(wrl::ComPtr<IMFByteStream> stream) = 0;
	// 流请求数据
	virtual concurrency::task<void> OnStreamsRequestData(TOperation& op) = 0;
	// 开始流
	virtual void OnStartStream(DWORD streamId, bool selected, REFERENCE_TIME position) = 0;
	virtual void OnPauseStream(DWORD streamId) = 0;
	virtual void OnStopStream(DWORD streamId) = 0;
private:
	bool HasShutdown() const;
	// 验证 Presentation Descriptor 有效
	void ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);
	// 创建 Presentation Descriptor
	concurrency::task<void> CreatePresentationDescriptor(wrl::ComPtr<IMFByteStream> stream);
	virtual void DispatchOperation(TOperation& op);
	virtual void ValidateOperation(TOperation& op);
	// 加载音源
	concurrency::task<void> InitializeAudioSource(wrl::ComPtr<IMFByteStream> stream);

	// 开始
	void DoStart(MediaSourceStartOperation* operation);
	void StartStreams(IMFPresentationDescriptor* pd, REFERENCE_TIME position);

	void DoPause();
	void PauseStreams();

	void DoStop();
	void StopStreams();

	// 流停止
	void OnEndOfStream();
protected:
	MFMediaSourceState state = MFMediaSourceState::NotInitialized;	// 状态
private:
	wrl::ComPtr<IMFMediaEventQueue> eventQueue;					// 事件队列
	std::recursive_mutex stateMutex;
	wrl::ComPtr<IMFPresentationDescriptor> presentDescriptor;	// PresentationDescriptor
	uint32_t pendingEOSCount = 0;								// 需要结束的流数量
};

NSED_TOMATO_MEDIA