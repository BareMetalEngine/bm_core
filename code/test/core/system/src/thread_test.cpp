/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/thread.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(Thread, StartsAndFinish)
{
	std::atomic<bool> started = false;

	{
		ThreadSetup setup;
		setup.m_function = [&started]() { started = true; };

		Thread t;
		t.init(setup);

		Thread::Sleep(100);
	}

	EXPECT_TRUE(started);
}

TEST(Thread, CleanFinish)
{
	std::atomic<bool> finished = false;

	{
		ThreadSetup setup;
		setup.m_function = [&finished]()
		{
			Thread::Sleep(100);
			finished = true;
		};

		Thread t;
		t.init(setup);
	}

	EXPECT_TRUE(finished);
}

TEST(Thread, HasUniqueThreadID)
{
	const auto currentThreadID = Thread::CurrentThreadID();
	EXPECT_NE(0, currentThreadID);

	{
		ThreadSetup setup;
		setup.m_function = [&currentThreadID]() {
			const auto localThreadID = Thread::CurrentThreadID();
			EXPECT_NE(currentThreadID, localThreadID);
		};

		Thread t;
		t.init(setup);
	}
}

//--

END_INFERNO_NAMESPACE();