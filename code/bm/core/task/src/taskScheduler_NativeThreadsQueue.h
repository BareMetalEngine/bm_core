/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "taskScheduler.h"

#include "bm/core/containers/include/groupQueue.h"
#include "bm/core/system/include/conditionVariable.h"
#include "bm/core/system/include/mutex.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// Queue used by the 
class TaskScheduler_NativeThreadsQueue : public NoCopy
{
public:
	TaskScheduler_NativeThreadsQueue();
	~TaskScheduler_NativeThreadsQueue();

	//--

	// add task to queue
	void scheduleTask(TaskEntry* entry);

	// pop work item, adds a reference to the task entry internal thread counter
	bool popTask(TaskEntry*& outEntry, uint32_t& outInstanceIndex);

	// signal work finished for given task entry
	void finishTask(TaskEntry* entry, uint32_t instanceIndex);

	//--

private:
	std::atomic<uint32_t> m_groupCounter;
	std::atomic<uint32_t> m_spinCounter;

	SpinLock m_queueLock;
	Semaphore m_queueSemaphore;

	GroupQueue m_queue;

	bool popTask_NoLock(TaskEntry*& outEntry, uint32_t& outInstanceInde);
};

//--

END_INFERNO_NAMESPACE()
