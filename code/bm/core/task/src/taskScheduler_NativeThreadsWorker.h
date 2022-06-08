/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "taskEntry.h"

BEGIN_INFERNO_NAMESPACE()

//--

class TaskScheduler_NativeThreadsQueue;
class TaskScheduler_NativeThreadEventPool;

//--

/// Worker thread for native scheduler
class TaskScheduler_NativeThreadWorker : public ITaskYielder
{
public:
	TaskScheduler_NativeThreadWorker(TaskScheduler_NativeThreadsQueue* taskQueue, TaskScheduler_NativeThreadEventPool* eventPool, uint32_t index, ThreadPriority prio, int affinity);
	~TaskScheduler_NativeThreadWorker();

	//--

private:
	StringID m_name;

	std::atomic<bool> m_requestExit;

	TaskScheduler_NativeThreadsQueue* m_queue = nullptr;
	TaskScheduler_NativeThreadEventPool* m_eventPool = nullptr;

	Thread m_thread;

	void threadFunc();

	virtual void yieldTaskAndWaitForSignal(TaskSignal signal) override final;
};

//--

END_INFERNO_NAMESPACE()
