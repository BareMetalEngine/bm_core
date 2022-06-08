/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "taskSignal.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

// a helper class that "builds" a task
class BM_CORE_TASK_API TaskBuilder : public NoCopy
{
public:
    TaskBuilder(StringID name = "Task"_id); // build async "floating task"
    TaskBuilder(TaskContext& c, StringID name = "Task"_id); // build a task that will be added to existing task group
    ~TaskBuilder();

    //--

    // change task scheduler
    TaskBuilder& scheduler(ITaskScheduler& scheduler);

    // specify number of task instances to run
    TaskBuilder& instances(uint32_t count);

    // Specify maximum task concurrency (only instanced tasks) - controls how many instances of this task may be running
    // NOTE: this takes into account the yielded ones
    TaskBuilder& concurency(uint32_t concurency); 

    //--

    // specify signals to wait for before task can be started
    TaskBuilder& waitFor(TaskSignal signal);
    TaskBuilder& waitFor(std::initializer_list<TaskSignal> signals);
    TaskBuilder& waitFor(ArrayView<TaskSignal> signals);

	// specify signals to trigger once task finishes
	TaskBuilder& signal(TaskSignal signal);
    TaskBuilder& signal(std::initializer_list<TaskSignal> signals);
    TaskBuilder& signal(ArrayView<TaskSignal> signals);

    //--

    // bind task function and send task for execution
    TaskSignal operator<<(TTaskFunc func);
	TaskSignal operator<<(TTaskFuncEx func);
	TaskSignal operator<<(TTaskInstancingFunc func);
	TaskSignal operator<<(TTaskInstancingFuncEx func);

    //--

protected:
    ITaskScheduler* m_scheduler = nullptr;
    TaskEntry* m_entry = nullptr;
    InplaceArray<TaskSignal, 16> m_waitForSignals;

    TaskSignal submit();
};

//--

END_INFERNO_NAMESPACE()

