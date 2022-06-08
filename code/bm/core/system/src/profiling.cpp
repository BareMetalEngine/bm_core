/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "profiling.h"

BEGIN_INFERNO_NAMESPACE()

//--

#ifdef WITH_PROFILING

class FrameProAllocator : public FramePro::Allocator
{
	virtual void* Alloc(size_t size) { return malloc(size); }
	virtual void Free(void* p) { return free(p); }
};

static FrameProAllocator GFrameProAllocator;
static bool GFrameProInitialized = false;

void InitProfiling()
{
	if (!GFrameProInitialized)
	{
		GFrameProInitialized = true;
		FRAMEPRO_SET_ALLOCATOR(&GFrameProAllocator);
		FRAMEPRO_SESSION_INFO("Build", "InfernoEngine v4");
		FRAMEPRO_SET_THREAD_NAME("MainThread");
	}
}

void InitProfilingThread(const char* name, int id, int affinity, int priority)
{
	if (GFrameProInitialized)
	{
		FRAMEPRO_SET_THREAD_ID_NAME(id, name);
		FRAMEPRO_SET_THREAD_PRIORITY(priority);
		if (affinity >= 0)
			FRAMEPRO_SET_THREAD_AFFINITY(affinity);
	}
}

void CloseProfilingThread()
{
	if (GFrameProInitialized)
	{
		FRAMEPRO_CLEANUP_THREAD();
	}
}

void CloseProfiling()
{
	if (GFrameProInitialized)
	{
		FRAMEPRO_SHUTDOWN();
	}
}

void MarkProfilingFrame()
{
	if (GFrameProInitialized)
	{
		FRAMEPRO_FRAME_START();
	}
}

#else

void InitProfiling() {};
void InitProfilingThread(const char* name, int id, int affinity, int priority) {};
void CloseProfilingThread() {};
void CloseProfiling() {};
void MarkProfilingFrame() {};

#endif

//--

END_INFERNO_NAMESPACE()
