/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(SpinLock, BasicLock)
{
	struct State
	{
		SpinLock lock;
		std::atomic<uint32_t> protectedData;
	};

	auto* state = new State();
	state->protectedData = 0;

	ThreadSetup setup;
	setup.m_function = [state]()
	{
		for (uint32_t i = 0; i < 100; ++i)
		{
			state->lock.acquire();

			auto val = state->protectedData++;
			EXPECT_EQ(0, val);
			Thread::YieldThread();
			auto val2 = --state->protectedData;
			EXPECT_EQ(0, val);

			state->lock.release();
		}
	};

	Thread threads[8];
	for (uint32_t i = 0; i < 8; ++i)
		threads[i].init(setup);
}

//--

END_INFERNO_NAMESPACE()