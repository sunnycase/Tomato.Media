//
// Tomato Media
// Media Foundation 音频流
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "MFMediaStreamState.h"
#include "MFSources/AudioSourceBase.h"
#include <queue>

NSDEF_TOMATO_MEDIA

class MFAudioStream : public wrl::RuntimeClass<wrl::RuntimeClassFlags<
	wrl::RuntimeClassType::ClassicCom>,
	IMFMediaEventGenerator,
	IMFMediaStream>
{
public:
	// 缓存的Sample数量
	const size_t PreRollSample = 1;

	MFAudioStream(AudioSourceBase* audioSource, wrl::ComPtr<IMFStreamDescriptor> streamDescriptor);
	virtual ~MFAudioStream();

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback *pCallback, IUnknown *punkState);
	STDMETHODIMP EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
	STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent);
	STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue);

	// IMFMediaStream
	STDMETHODIMP GetMediaSource(IMFMediaSource **ppMediaSource);
	STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor **ppStreamDescriptor);
	STDMETHODIMP RequestSample(IUnknown *pToken);

	// 提示到达传送结尾
	void EndOfDeliver();
	// 传送采样
	void DeliverPayload(IMFSample* sample);
	// 获取状态
	MFMediaStreamState GetState() const noexcept;
	// 启动
	void Start(REFERENCE_TIME position);
	// 暂停
	void Pause();
	// 停止
	void Stop();
	void Active(bool active);
	bool IsActived() const noexcept { return actived; }
private:
	// 是否需要更多数据
	bool DoesNeedMoreData();

	// 请求采样数据
	void DispatchSampleRequests();
	void OnEndOfStream();
protected:
	MFMediaStreamState state = MFMediaStreamState::NotInitialized;		// 状态
private:
	bool endOfDeliver;
	wrl::ComPtr<IMFMediaEventQueue> eventQueue;			// 事件队列
	AudioSourceBase* audioSource;						// 音频源
	wrl::ComPtr<IMFStreamDescriptor> streamDescriptor;	// Stream Descriptor

	std::queue<wrl::ComPtr<IMFSample>> samplesCache;		// 采样缓存
	std::queue<wrl::ComPtr<IUnknown>> sampleRequests;		// 采样请求
	std::recursive_mutex sampleMutex;
	bool actived = false;
};

NSED_TOMATO_MEDIA