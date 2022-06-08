/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/timing.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(NativeTimePoint, NowNotZero)
{
	auto now = NativeTimePoint::Now();
	EXPECT_NE(0, now.rawValue());
}

TEST(NativeTimePoint, StrictlyMonotonic)
{
	NativeTimePoint prev;
	for (int i = 0; i < 100; ++i)
	{
		Thread::Sleep(1);
		auto now = NativeTimePoint::Now();
		if (i > 1)
			EXPECT_LT(prev, now);
	}
}

TEST(NativeTimePoint, MatchesRealTime)
{
	auto then = NativeTimePoint::Now();
	Thread::Sleep(100);
	auto now = NativeTimePoint::Now();

	float passed = (now - then).toSeconds();
	EXPECT_TRUE(passed > 0.09f && passed <= 0.2f);
}

TEST(NativeTimePoint, NonDestructive)
{
	auto then = NativeTimePoint::Now();
	Thread::Sleep(10);
	auto now = NativeTimePoint::Now();

	auto interval = (now - then);
	auto now2 = then + interval;
	EXPECT_TRUE(now == now2);
	auto then2 = now - interval;
	EXPECT_TRUE(then == then2);
}

/*TEST(NativeTimePoint, NonDestructiveWithDouble)
{
	auto then = NativeTimePoint::Now();
	Thread::Sleep(10);
	auto now = NativeTimePoint::Now();

	auto interval = (now - then).toSeconds();
	auto now2 = then + interval;
	EXPECT_TRUE(now == now2);
	auto then2 = now - interval;
	EXPECT_TRUE(then == then2);
}*/

//--

END_INFERNO_NAMESPACE()