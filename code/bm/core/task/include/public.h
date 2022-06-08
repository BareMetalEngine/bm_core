/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm_core_task_glue.inl"

BEGIN_INFERNO_NAMESPACE()

//--

class ITaskScheduler;

struct TaskSignal;
struct TaskContext;
struct TaskEntry;

typedef std::function<void(void)> TSignalCompletionCallback;

typedef std::function<void(TaskContext& tc)> TTaskFunc;
typedef std::function<void(TaskContext& tc, uint32_t index)> TTaskInstancingFunc;
typedef std::function<void()> TTaskFuncEx;
typedef std::function<void(uint32_t index)> TTaskInstancingFuncEx;

typedef std::function<void(IndexRange range)> TTaskForFunc;
typedef std::function<void(TaskContext& tc, IndexRange range)> TTaskForFuncEx;

//--

// initialize thread system
extern BM_CORE_TASK_API bool InitTaskThreads(const CommandLine& cmdLine);

// shut down thread system
extern BM_CORE_TASK_API void CloseTaskThreads();

//--

// non-task task context (yields via system event, creates bubbles)
extern BM_CORE_TASK_API TaskContext& NoTask();

// maximum task concurency level (number of worker threads)
extern BM_CORE_TASK_API uint32_t MaxTaskConcurency();

//--

END_INFERNO_NAMESPACE()
