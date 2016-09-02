//
// Tomato Media
// Media Foundation 工作队列提供程序
// 
// 作者：SunnyCase
// 创建日期 2015-05-12
#pragma once
#include <Tomato.Media/Tomato.Media.h>
#include <Tomato.Core/WorkerQueueProvider.h>
#include <Tomato.Core/NonCopyable.h>

DEFINE_NS_MEDIA

#pragma warning(push)
#pragma warning(disable: 4275)

class TOMATO_MEDIA_API MFWorkerQueueProviderRef : public Core::WorkerQueueProvider
{
public:
	explicit MFWorkerQueueProviderRef(DWORD queueId) noexcept;

	///<summary>获取此工作队列是否有效</summary>
	bool IsValid() const noexcept;

	virtual std::shared_ptr<Core::WorkerThread> CreateWorkerThread(std::function<void()>&& callback) override;
private:
	DWORD queueId;
};

class TOMATO_MEDIA_API MFWorkerQueueProvider : public Core::WorkerQueueProvider, Core::NonCopyable
{
public:
	// 获取 Pro Audio 工作队列
	static MFWorkerQueueProviderRef GetAudio();
	// 获取 Pro Audio 工作队列
	static MFWorkerQueueProviderRef GetProAudio();

	MFWorkerQueueProvider(DWORD taskId, LPCWSTR className, DWORD basePriority);
	virtual ~MFWorkerQueueProvider();

	DWORD GetTaskId() const noexcept { return taskId; }

	virtual std::shared_ptr<Core::WorkerThread> CreateWorkerThread(std::function<void()>&& callback) override;
	// 创建串行工作队列
	std::unique_ptr<Core::WorkerQueueProvider> CreateSerial();
private:
	DWORD taskId, queueId;
};

#pragma warning(pop)

END_NS_MEDIA