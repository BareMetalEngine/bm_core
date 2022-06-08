/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/locklessPool.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(LocklessPool, BasicTest)
{
	typedef LockLessPool<int, 256, uint8_t> TPool;

	TPool pool;

	for (uint32_t i = 0; i < 7; ++i)
	{
		const auto BATCH_SIZE = 1U << i;

		for (uint32_t j = 0; j < 10; ++j)
		{
			int* items[256];
			for (uint32_t k = 0; k < BATCH_SIZE; ++k)
				items[i] = pool.alloc();

			for (uint32_t k = 0; k < BATCH_SIZE; ++k)
				pool.free(items[k]);
		}
	}
}

//--

END_INFERNO_NAMESPACE()