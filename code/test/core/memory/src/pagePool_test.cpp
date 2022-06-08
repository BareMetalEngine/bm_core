/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/memory/include/poolPaged.h"

BEGIN_INFERNO_NAMESPACE()

//---

TEST(PagedPool, ReturnsPow2Page)
{
	auto page = LocalPagePool().allocatPage(4096);
	ASSERT_NE(nullptr, page.basePtr);
	EXPECT_TRUE(IsPowerOf2(page.size()));
	LocalPagePool().freePage(page);
}

TEST(LocalAllocator, PagesUniqueIndices)
{
	auto page = LocalPagePool().allocatPage(4096);
	auto page2 = LocalPagePool().allocatPage(4096);
	EXPECT_NE(page.index, page2.index);
	LocalPagePool().freePage(page);
	LocalPagePool().freePage(page2);
}

TEST(LocalAllocator, PagesUniqueIndicesAfterFree)
{
	auto page = LocalPagePool().allocatPage(4096);
	auto index = page.index;
	LocalPagePool().freePage(page);

	auto page2 = LocalPagePool().allocatPage(4096);
	auto index2 = page2.index;
	LocalPagePool().freePage(page2);

	ASSERT_NE(index, index2);
}

TEST(LocalAllocator, DoubleFreeDoesNotCrash)
{
	auto page = LocalPagePool().allocatPage(4096);
	LocalPagePool().freePage(page);
	//LocalPagePool().freePage(page);
	// TODO
}

//---


END_INFERNO_NAMESPACE()