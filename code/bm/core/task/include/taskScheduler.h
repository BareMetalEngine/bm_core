/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// task scheduler interface
class BM_CORE_TASK_API ITaskScheduler : public NoCopy
{
public:
	virtual ~ITaskScheduler();

	/// schedule task for execution
	virtual void scheduleTask(TaskEntry* entry) = 0;
};

//--

// main task scheduler
extern BM_CORE_TASK_API ITaskScheduler& MainScheduler();

// background task scheduler - few low priority threads for longer jobs 
extern BM_CORE_TASK_API ITaskScheduler& BackgroundScheduler();

//--

END_INFERNO_NAMESPACE()
