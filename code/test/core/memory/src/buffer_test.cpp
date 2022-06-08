/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/memory/include/buffer.h"
#include "bm/core/memory/include/poolUnmanaged.h"
#include "proxyPools.h"

BEGIN_INFERNO_NAMESPACE()

//---

TEST(Buffer, CreatedReportsAsEmpty)
{
	Buffer buf;
	EXPECT_TRUE(buf.empty());
}

TEST(Buffer, CreatedReportsZeroSize)
{
	Buffer buf;
	EXPECT_EQ(0, buf.size());
}

TEST(Buffer, CreatedReportsEmptyView)
{
	Buffer buf;
	EXPECT_TRUE(buf.view().empty());
}

TEST(Buffer, CreatedReportsViewWithSizeZero)
{
	Buffer buf;
	EXPECT_EQ(0, buf.view().size());
}

//---

TEST(Buffer, EmptySubViewOfEmptyBufferWorks)
{
	Buffer buf;
	Buffer subBuf = buf.createSubBuffer(0, 0);
	EXPECT_TRUE(subBuf.empty());
}

TEST(Buffer, BufferZeroRequestCreatesEmptyBuffer)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 0);
	EXPECT_TRUE(buf.empty());
	EXPECT_EQ(nullptr, buf.data());
}

TEST(Buffer, BufferAllocatesWhenRequested)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 128, 8);
	EXPECT_NE(nullptr, buf.data());
}

TEST(Buffer, BufferAllocatesInOneAllocation)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 128, 8);
	EXPECT_EQ(1, pool.m_countedAllocations);
}

TEST(Buffer, BufferHasPropperAlignment)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	EXPECT_TRUE(IsAligned(buf.data(), 16));
}

TEST(Buffer, BufferReportsFullView)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	EXPECT_EQ(177, buf.view().size());
}

TEST(Buffer, BufferFreesIntoPool)
{
	prv::ProxyUnmanagedPool pool;

	{
		auto buf = Buffer::CreateEmpty(pool, 177, 16);
	}

	EXPECT_EQ(1, pool.m_countedAllocations);
	EXPECT_EQ(1, pool.m_countedFree);
	EXPECT_EQ(0, pool.m_activeAllocations);
}

TEST(Buffer, BufferEmptyAfterReset)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	buf.reset();

	EXPECT_TRUE(buf.empty());
}

TEST(Buffer, BufferHasZeroSizeAfterReset)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	buf.reset();

	EXPECT_EQ(0, buf.size());
}

TEST(Buffer, BufferHasEmptyViewAfterReset)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	buf.reset();

	EXPECT_TRUE(buf.view().empty());
}

TEST(Buffer, BufferResetsIntoPool)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	buf.reset();

	EXPECT_EQ(1, pool.m_countedAllocations);
	EXPECT_EQ(1, pool.m_countedFree);
	EXPECT_EQ(0, pool.m_activeAllocations);
}

TEST(Buffer, ExternalBufferPreservesPointer)
{
	auto buf = Buffer::CreateExternal(BufferView((void*)0x1234, 1000), [](void* ptr) {});
	EXPECT_EQ((void*)0x1234, buf.data());
}

TEST(Buffer, ExternalBufferPreservesSize)
{
	auto buf = Buffer::CreateExternal(BufferView((void*)0x1234, 1000), [](void* ptr) {});
	EXPECT_EQ(1000, buf.size());
}

TEST(Buffer, ExternalBufferCallsCallbackOnFree)
{
	bool callbackCalled = false;
	{
		auto buf = Buffer::CreateExternal(BufferView((void*)0x1234, 1000), [&callbackCalled](void* ptr) { callbackCalled = true; });
		EXPECT_FALSE(callbackCalled);
	}
	EXPECT_TRUE(callbackCalled);
}

//---

TEST(Buffer, BufferSubViewCreatesProperly)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);

	auto subBuf = buf.createSubBuffer(20, 10);
	EXPECT_FALSE(subBuf.empty());
}

TEST(Buffer, BufferSubViewHasProperSize)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);

	auto subBuf = buf.createSubBuffer(20, 10);
	EXPECT_EQ(10, subBuf.size());
}

TEST(Buffer, BufferSubViewHasProperPointer)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);

	auto subBuf = buf.createSubBuffer(20, 10);

	auto* ptr = buf.data();
	auto* subPtr = subBuf.data();

	EXPECT_EQ(ptr + 20, subPtr);
}

TEST(Buffer, BufferSubViewDoesNotAllocate)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	auto subBuf = buf.createSubBuffer(20, 10);
	EXPECT_EQ(1, pool.m_activeAllocations);
}

TEST(Buffer, BufferSubViewKeepsBufferAlive)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	auto subBuf = buf.createSubBuffer(10, 10);
	buf.reset();

	EXPECT_EQ(0, pool.m_countedFree);
	EXPECT_EQ(1, pool.m_activeAllocations);

	subBuf.reset();

	EXPECT_EQ(1, pool.m_countedFree);
	EXPECT_EQ(0, pool.m_activeAllocations);
}

TEST(Buffer, BufferSubViewSeesSameData)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 177, 16);
	memset(buf.data(), 0, buf.size());
	buf.data()[15] = 42;

	auto subBuf = buf.createSubBuffer(10, 20);
	EXPECT_EQ(42, subBuf.data()[5]);

	buf.data()[15] = 123;
	EXPECT_EQ(123, subBuf.data()[5]);
}

TEST(Buffer, BufferSubViewKeepsBufferAliveExternalDoesNotCallback)
{
	prv::ProxyUnmanagedPool pool;

	bool callbackCalled = false;
	auto buf = Buffer::CreateExternal(BufferView((void*)0x1234, 1000), [&callbackCalled](void* ptr) { callbackCalled = true; });
	auto subBuf = buf.createSubBuffer(10, 10);
	buf.reset();

	EXPECT_FALSE(callbackCalled);

	subBuf.reset();

	EXPECT_TRUE(callbackCalled);
}

//---

namespace prv
{
	static uint8_t ValueAtOffset(uint32_t offset)
	{
		return 0x30 + offset;
	}

	static void FillBuffer(BufferView view)
	{
		for (uint32_t i = 0; i < view.size(); ++i)
			view.data()[i] = ValueAtOffset(i);
	}
} // prv

TEST(Buffer, BufferCopyCreateCopy)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view());
	EXPECT_FALSE(copyBuf.empty());
}

TEST(Buffer, BufferCopyCreateCopyHasProperSize)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view());
	EXPECT_EQ(buf.size(), copyBuf.size());
}

TEST(Buffer, BufferCopyIsAllocated)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view());
	EXPECT_EQ(2, pool.m_activeAllocations);
}

TEST(Buffer, BufferCopyHasSameData)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);
	memset(buf.data(), 0, buf.size());
	buf.data()[20] = 42;

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view());
	EXPECT_EQ(42, copyBuf.data()[20]);
}

TEST(Buffer, BufferCopyWithOffsetHasProperSize)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);
	memset(buf.data(), 0, buf.size());
	buf.data()[20] = 42;

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view().subView(10, 20));
	EXPECT_EQ(20, copyBuf.size());
}

TEST(Buffer, BufferCopyWithOffsetCopiesPartOfData)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);
	memset(buf.data(), 0, buf.size());
	buf.data()[20] = 42;

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view().subView(10, 20));
	EXPECT_EQ(42, copyBuf.data()[10]);
}

TEST(Buffer, BufferCopyDoesNotSeeChanges)
{
	prv::ProxyUnmanagedPool pool;

	auto buf = Buffer::CreateEmpty(pool, 64);
	memset(buf.data(), 0, buf.size());
	buf.data()[20] = 42;

	auto copyBuf = Buffer::CreateFromCopy(pool, buf.view());
	EXPECT_EQ(42, copyBuf.data()[20]);

	buf.data()[20] = 123;

	EXPECT_EQ(42, copyBuf.data()[20]);
}

TEST(Buffer, BufferFromAllreadyAllocatedPreservesPointer)
{
	prv::ProxyUnmanagedPool pool;

	auto memory = pool.allocateMemory(100, 4);

	auto buf = Buffer::CreateFromAlreadyAllocatedMemory(pool, BufferView(memory, 100));
	EXPECT_EQ(buf.data(), memory);
}

TEST(Buffer, BufferFromAllreadyAllocatedPreservesSize)
{
	prv::ProxyUnmanagedPool pool;

	auto memory = pool.allocateMemory(100, 4);

	auto buf = Buffer::CreateFromAlreadyAllocatedMemory(pool, BufferView(memory, 100));
	EXPECT_EQ(100, buf.size());
}

TEST(Buffer, BufferFromAllreadyAllocatedDoesNotAllocateMore)
{
	prv::ProxyUnmanagedPool pool;

	auto memory = pool.allocateMemory(100, 4);
	EXPECT_EQ(1, pool.m_activeAllocations);

	auto buf = Buffer::CreateFromAlreadyAllocatedMemory(pool, BufferView(memory, 100));
	EXPECT_EQ(1, pool.m_activeAllocations);
}

TEST(Buffer, BufferFromAllreadyAllocatedReturnsToPool)
{
	prv::ProxyUnmanagedPool pool;

	{
		auto memory = pool.allocateMemory(100, 4);
		auto buf = Buffer::CreateFromAlreadyAllocatedMemory(pool, BufferView(memory, 100));
	}

	EXPECT_EQ(0, pool.m_activeAllocations);
}

END_INFERNO_NAMESPACE()