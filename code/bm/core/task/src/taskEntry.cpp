/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskEntry.h"
#include "bm/core/memory/include/structureAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//--

ITaskYielder::~ITaskYielder()
{}

//--

struct TaskEntryPool
{
	SpinLock m_entiresLock;
	StructureAllocator<TaskEntry> m_entires;

	TaskEntryPool()
		: m_entires(sizeof(TaskEntry) * 4096)
	{}

	TaskEntry* alloc()
	{
		auto lock = CreateLock(m_entiresLock);
		return m_entires.create();
	}

	void free(TaskEntry* ptr)
	{
		auto lock = CreateLock(m_entiresLock);
		return m_entires.free(ptr);
	}
};

static TaskEntryPool* GTaskEntryPool = new TaskEntryPool();

//--

TaskEntry* TaskEntry::Alloc()
{
	return GTaskEntryPool->alloc();
}

void TaskEntry::Free(TaskEntry* entry)
{
	GTaskEntryPool->free(entry);
}

//--

struct FakeTaskContext : public TaskContext, public ITaskYielder
{
	FakeTaskContext()
	{
		groupIndex = 0;
		yielder = this;
	}

	virtual void yieldTaskAndWaitForSignal(TaskSignal signal) override
	{
		Event evt(EventType::ManualReset);

		signal.registerCompletionCallback([&evt]()
			{
				evt.trigger();
			});

		evt.waitInfinite(); // YIELDS THE THREAD
	}
};

TaskContext& NoTask()
{
	static FakeTaskContext theContext;
	return theContext;
}

//--

END_INFERNO_NAMESPACE()
