/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/memory/include/localAllocator.h"
#include "bm/core/memory/include/poolUnmanaged.h"
#include "bm/core/memory/include/poolPaged.h"
#include "proxyPools.h"

BEGIN_INFERNO_NAMESPACE()

//---

class DebugPagePool : public IPoolPaged
{
public:
	DebugPagePool(IPoolPaged& pool)
		: IPoolPaged("DebugPagePool")
		, m_pool(pool)
	{}

	virtual MemoryPage allocatPage(uint64_t size) override final
	{
		auto ret = m_pool.allocatPage(size);
		m_numAllocBytes += ret.size();
		m_numAllocCalled += 1;
		return ret;
	}

	virtual void freePage(MemoryPage page) override final
	{
		m_numFreeCalled += 1;
		m_numFreeBytes += page.size();
		return m_pool.freePage(page);
	}

	virtual uint64_t queryMinimumPageSize() const override final
	{
		return m_pool.queryMinimumPageSize();
	}

	virtual uint64_t queryMaximumPageSize() const override final
	{
		return m_pool.queryMaximumPageSize();
	}

	uint32_t m_numFreeCalled = 0;
	uint32_t m_numFreeBytes = 0;

	uint32_t m_numAllocCalled = 0;
	uint32_t m_numAllocBytes = 0;

private:
	IPoolPaged& m_pool;
};

//---

TEST(LocalAllocator, InitializesToEmpty)
{
	LocalAllocator mem;
	EXPECT_EQ(0, mem.stats().numAllocatedBytes);
}

TEST(LocalAllocator, SimpleAllocationVisible)
{
	LocalAllocator mem;
	auto* data = mem.alloc(16, 4);
	EXPECT_EQ(16, mem.stats().numAllocatedBytes);
	EXPECT_EQ(1, mem.stats().numBlocks);
}

struct ObjectCounter
{
public:
	static uint32_t GCount;

	ObjectCounter() { GCount++; }
	~ObjectCounter() { GCount--; }
};

uint32_t ObjectCounter::GCount = 0;

TEST(LocalAllocator, DestructorCalledWhenRegistered)
{
	{
		LocalAllocator mem;
		auto* obj = mem.createAndRegisterDestroyer<ObjectCounter>();
		EXPECT_EQ(1, mem.stats().numDestroyers);
		EXPECT_EQ(1, ObjectCounter::GCount);
	}
	EXPECT_EQ(0, ObjectCounter::GCount);
}

TEST(LocalAllocator, DestructorNotCalledWhenNotRegistered)
{
	{
		LocalAllocator mem;
		auto* obj = mem.createWithoutDestruction<ObjectCounter>();
		EXPECT_EQ(0, mem.stats().numDestroyers);
		EXPECT_EQ(1, ObjectCounter::GCount);
	}
	EXPECT_EQ(1, ObjectCounter::GCount);
	ObjectCounter::GCount = 0;
}

//---

END_INFERNO_NAMESPACE()