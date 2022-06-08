/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "pool.h"

BEGIN_INFERNO_NAMESPACE()

//--

namespace prv
{

	class InternalPoolRegistry : public MainPoolData<NoCopy>
	{
	public:
		InternalPoolRegistry()
		{
			memzero(&m_pools, sizeof(m_pools));
		}

		uint32_t registerPool(IPool* pool)
		{
			auto lock = CreateLock(m_lock);

			for (uint32_t i = 1; i < MAX_POOLS; ++i)
			{
				if (!m_pools[i])
				{
					m_pools[i] = pool;
					return i;
				}
			}

			ASSERT(!"To many pools registered");
			return 0;
		}

		void unregisterPool(IPool* pool, uint32_t index)
		{
			auto lock = CreateLock(m_lock);

			ASSERT_EX(m_pools[index] == pool, "Invalid pool at index");
			m_pools[index] = nullptr;
		}

		void iteratePools(const std::function<void(IPool&)>& func)
		{
			auto lock = CreateLock(m_lock);

			for (uint32_t i = 1; i < MAX_POOLS; ++i)
				if (m_pools[i])
					func(*m_pools[i]);
		}

		static InternalPoolRegistry& GetInstance()
		{
			static InternalPoolRegistry theInstance;
			return theInstance;
		}
		
	private:
		static const uint32_t MAX_POOLS = 256;

		SpinLock m_lock;
		IPool* m_pools[MAX_POOLS];
	};

} // prv

//--

PoolStats::PoolStats()
{}

void PoolStats::print(IFormatStream& f) const
{
	if (totalSize)
		f.appendf("TotalSize: {}", MemSize(totalSize));

	if (wastedBytes)
		f.appendf("CurAllocated: {} ({} blocks), {} wasted ({} of total)", MemSize(allocatedBytes), allocatedBlocks, MemSize(wastedBytes), Percent(wastedBytes, allocatedBytes));
	else
		f.appendf("CurAllocated: {} ({} blocks)", MemSize(allocatedBytes), allocatedBlocks);
	f.appendf("MaxAllocated: {} ({} blocks)", MemSize(maxAllocatedBytes), maxAllocatedBlocks);
	f.appendf("IncAllocated: {} ({} blocks)", MemSize(runningAllocationSize), runningAllocationCount);
}

//--

IPool::IPool(const char* name, PoolType type)
	: m_name(name)
	, m_type(type)
	, m_statTotalSize(0)
	, m_statAllocatedBytes(0)
	, m_statAllocatedBlocks(0)
	, m_statMaxAllocatedBytes(0)
	, m_statMaxAllocatedBlocks(0)
	, m_statAllocationCounter(0)
	, m_statAllocationByteCounter(0)
{
	m_index = prv::InternalPoolRegistry::GetInstance().registerPool(this);
}

IPool::~IPool()
{
	if (m_index)
	{
		prv::InternalPoolRegistry::GetInstance().unregisterPool(this, m_index);
		m_index = 0;
	}
}

void IPool::IteratePools(const std::function<void(IPool&)>& func)
{
	prv::InternalPoolRegistry::GetInstance().iteratePools(func);
}

void IPool::PrintPools(IFormatStream& f, int details)
{
	bool hasDivider = false;

	prv::InternalPoolRegistry::GetInstance().iteratePools([&f, &hasDivider, details](IPool& p)
		{
			TempString title;
			title << "==[ ";
			title << p.name();
			title << " ]";
			while (title.length() < 70)
				title.append("=");
			f << title << "\n";

			p.print(f, details);
			hasDivider = true;
		});

	if (hasDivider)
	{
		f.append("======================================================================\n");
		f.append("\n");
	}
}

//---

void IPool::stats(PoolStats& outStats) const
{
	outStats.totalSize = m_statTotalSize.load();
	outStats.allocatedBytes = m_statAllocatedBytes.load();
	outStats.allocatedBlocks = m_statAllocatedBlocks.load();
	outStats.maxAllocatedBytes = m_statMaxAllocatedBytes.load();
	outStats.maxAllocatedBlocks = m_statMaxAllocatedBlocks.load();
	outStats.runningAllocationCount = m_statAllocationCounter.load();
	outStats.runningAllocationSize = m_statAllocationByteCounter.load();
}

void IPool::print(IFormatStream& f, int details /*= 0*/) const
{
	PoolStats st;
	stats(st);

	f.appendf(" Utilization: {}%\n", Percent(st.allocatedBytes, st.maxAllocatedBytes));
	f.appendf(" Current: {} ({} blocks)\n", MemSize(st.allocatedBytes), st.allocatedBlocks);
	f.appendf(" Maximum: {} ({} blocks)\n", MemSize(st.maxAllocatedBytes), st.maxAllocatedBlocks);
}

void IPool::notifyTotalChange(int64_t change)
{
	m_statTotalSize += change;
}

void IPool::notifyAllocation(uint64_t size)
{
	m_statAllocationByteCounter += 1;
	m_statAllocationByteCounter += size;

	auto numBlocks = m_statAllocatedBlocks += 1;
	auto numBytes = m_statAllocatedBytes += size;
	UpdateMaximum(m_statMaxAllocatedBlocks, numBlocks + 1);
	UpdateMaximum(m_statMaxAllocatedBytes, numBytes + size);
}

void IPool::notifyFree(uint64_t size)
{
	m_statAllocatedBlocks -= 1;
	m_statAllocatedBytes -= size;
}

//--

END_INFERNO_NAMESPACE()
