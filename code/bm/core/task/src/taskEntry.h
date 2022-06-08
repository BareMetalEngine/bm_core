/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "taskSignal.h"

BEGIN_INFERNO_NAMESPACE()

//--

// Task yielder
class ITaskYielder : public NoCopy
{
public:
	virtual ~ITaskYielder();
	virtual void yieldTaskAndWaitForSignal(TaskSignal signal) = 0;
};

// Definition is hidden here to prevent client code from faking it to much
// The core idea is that if function behavior needs to YIELD the task we must know it and prepare for it (basically similar to "C# async")
struct TaskContext : public NoCopy
{
	uint32_t groupIndex = 0;
	ITaskYielder* yielder = nullptr;
	TaskSignal completionSignal = nullptr;
};

//--

// internal master structure describing live task entry
TYPE_ALIGN(64, struct) TaskEntry : public NoCopy
{
	uint32_t group = 0; // task group we belong to

	uint32_t concurency = std::numeric_limits<uint32_t>::max();
	uint32_t instances = 1; // how many times to run the task (instancing)

	StringID name; // task name
	TTaskInstancingFunc func; // task function

	TaskSignal signal; // signal to trip when task is finished

	std::atomic<uint32_t> scheduledJobs = 0; // count so far
	std::atomic<uint32_t> activeJobs; // number of active jobs running for this task entry
	std::atomic<uint32_t> remainingJobs; // counts towards zero to activate signal

	//--

	static TaskEntry* Alloc();
	static void Free(TaskEntry* entry);
};

//--

END_INFERNO_NAMESPACE()
