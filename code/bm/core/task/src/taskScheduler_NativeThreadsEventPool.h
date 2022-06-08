/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/system/include/event.h"
#include "bm/core/memory/include/structureAllocator.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// Pool of reusable events single shot events
class TaskScheduler_NativeThreadEventPool : public NoCopy
{
public:
	TaskScheduler_NativeThreadEventPool();
	~TaskScheduler_NativeThreadEventPool();

	//--

	Event* alloc();

	void free(Event* evt);

	//--

private:
	SpinLock m_lock;

	StructureAllocator<Event> m_allocator;
	InplaceArray<Event*, 256> m_freeEvents;
};

//--

END_INFERNO_NAMESPACE()
