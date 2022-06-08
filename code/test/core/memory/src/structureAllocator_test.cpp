/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/memory/include/structureAllocator.h"

BEGIN_INFERNO_NAMESPACE()

namespace test
{

	struct SmallData
	{
		uint8_t m_data[32];
	};

	TYPE_ALIGN(16, struct) AlignedElement
	{
		uint8_t m_data[32];
	};

} // test

TEST(StructureAllocator, Empty)
{
    StructureAllocator<test::SmallData> pool;
	EXPECT_EQ(0, pool.size());
	//EXPECT_EQ(0, pool.capacity());
	//EXPECT_TRUE(pool.empty());
}

TEST(StructureAllocator, AllocatesSingle)
{
    StructureAllocator<test::SmallData> pool;

	auto elem  = pool.create();
	EXPECT_EQ(1, pool.size());

	pool.free(elem);
	EXPECT_EQ(0, pool.size());
}

TEST(StructureAllocator, AllocatesDifferent)
{
    StructureAllocator<test::SmallData> pool;

    auto elem = pool.create();
    auto elem2 = pool.create();
    EXPECT_NE(elem, elem2);

    pool.free(elem);
    pool.free(elem2);
}

TEST(StructureAllocator, AllocatesMultiple)
{
    StructureAllocator<test::SmallData> pool;

	auto elem  = pool.create();
	EXPECT_EQ(1, pool.size());

	auto elem2  = pool.create();
	EXPECT_EQ(2, pool.size());

    pool.free(elem);
    pool.free(elem2);
    EXPECT_EQ(0, pool.size());
}

TEST(StructureAllocator, AlignedElement)
{
	StructureAllocator<test::AlignedElement> pool;

	auto elem  = pool.create();
	EXPECT_EQ(0, ((uint64_t)elem) & 15);

    pool.free(elem);
}

END_INFERNO_NAMESPACE()