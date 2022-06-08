/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileMapping.h"
#include "bm/core/task/include/taskSignal.h"
#include "bm/core/task/include/taskBuilder.h"
#include "bm/core/containers/include/queue.h"
#include "bm/core/memory/include/structureAllocator.h"

BEGIN_INFERNO_NAMESPACE()

#if 0

//--

class BudgetMemoryPool : public NoCopy
{
public:
	BudgetMemoryPool(uint64_t budget)
		: m_pendingAllocationsPool(sizeof(PendingAllocation*) * 1024)
		, m_budget(budget)
	{}

	void* alloc(TaskContext& tc, uint32_t size)
	{
		auto lock = CreateLock(m_lock);

		if (m_allocted + size < m_budget)
		{
			m_allocted += size;
			lock.release();
			return PoolAllocate(MainPool(), size, 16);
		}

		auto signal = TaskSignal::Create("PendingAllocation"_id);

		auto* info = m_pendingAllocationsPool.create();
		info->signal = signal;
		info->size = size;
		info->result = nullptr;

		m_pendingAllocations.push(info);

		lock.release();

		{
			PC_SCOPE_LVL2(WaitForMemory);
			signal.waitWithYeild(tc);
		}

		auto* ptr = info->result;
		m_pendingAllocationsPool.free(info);

		return ptr;
	}

	void free(void* ptr, uint32_t size)
	{
		auto lock = CreateLock(m_lock);
		m_allocted -= size;
	//	TRACE_INFO("[MEM] Free -{} -> {}", size, m_allocted);

		PoolFree(MainPool(), ptr);

		while (!m_pendingAllocations.empty())
		{
			auto* top = m_pendingAllocations.top();
			if (m_allocted + top->size > m_budget)
				break; // still don't fit

			m_pendingAllocations.pop();

			top->result = PoolAllocate(MainPool(), top->size, 16);
			top->signal.trip();

			m_allocted += top->size;			
		}
	}

private:
	SpinLock m_lock;

	uint64_t m_budget = 0;
	uint64_t m_allocted = 0;

	struct PendingAllocation
	{
		TaskSignal signal;
		uint32_t size = 0;
		void* result = nullptr;
	};

	Queue<PendingAllocation*> m_pendingAllocations;
	StructureAllocator<PendingAllocation> m_pendingAllocationsPool;
};

#if 1

#if 0
TEST(AsyncFile, BigReadIntoOneBlock)
{
	ScopeTimer timer;

	auto buffer = FileSystem().loadFileToBuffer("Z:/unity_source_code.rar");
	ASSERT_TRUE(!!buffer);

	const auto readTime = timer.timeElapsedInterval();

	auto size = buffer.size();
	TRACE_INFO("File size: {}", MemSize(size));

	const auto crcPageSize = 128ULL << 10;
	const auto crcPageCount = (size + crcPageSize - 1) / crcPageSize;

	ThreadSafe<Array<std::pair<uint64_t, uint64_t>>> results;

	auto sig = TaskBuilder("CRCTest"_id).instances(crcPageCount) << [&buffer, crcPageSize, size, &results](TaskContext& tc, uint32_t index)
	{
		const auto crcStart = (index * crcPageSize);
		const auto crcEnd = std::min<uint64_t>(size, crcStart + crcPageSize);

		auto crc = CRC64().append(buffer.data() + crcStart, crcEnd - crcStart).crc();
		results.lock()->emplaceBack(std::make_pair(crcStart, crc));
	};

	while (!sig.waitSpinWithTimeout(33))
	{
		MarkProfilingFrame();
	}

	results.lock()->sort([](auto a, auto b)
		{
			return a.first < b.first;
		});

	CRC64 finalResult;
	for (const auto p : *results.lock())
		finalResult << p.second;

	TRACE_WARNING("Final result: {} after {} ({} being read)", Hex(finalResult.crc()), timer, readTime);
}
#else
TEST(AsyncFile, BigReadWithTasks)
{
	ScopeTimer timer;

	auto file = FileSystem().openForReading("Z:/unity_source_code.rar", FileReadMode::DirectNonBuffered);
	ASSERT_TRUE(!!file);

	auto size = file->size();
	TRACE_INFO("File size: {}", MemSize(size));

	const auto crcPageSize = 128ULL << 10;

	const auto pageSize = crcPageSize * 8;
	const auto pageCount = (size + pageSize - 1) / pageSize;

	BudgetMemoryPool memoryPool(10000LLU << 20);

	ThreadSafe<Array<std::pair<uint64_t, uint64_t>>> results;
	ThreadSafe<Array<TaskSignal>> pendingSignals;

	auto sig = TaskBuilder("IOTest"_id).instances(pageCount).concurency(2) << [file, size, crcPageSize, pageSize, &memoryPool, &results, &pendingSignals](TaskContext& tc, uint32_t index)
	{
		const auto readStart = (index * pageSize);
		const auto readEnd = std::min<uint64_t>(size, pageSize + readStart);

		const auto readSize = readEnd - readStart;
		auto* memory = (uint8_t*)memoryPool.alloc(tc, readSize);

		uint32_t actualReadSize = 0;;
		if (file->readAsync(tc, FileAbsoluteRange(readStart, readEnd), memory, actualReadSize))
		{
			const auto crcPageCount = (actualReadSize + crcPageSize - 1) / crcPageSize;

			auto sig = TaskBuilder("CRCTest"_id).instances(crcPageCount) << [memory, actualReadSize, readStart, crcPageSize, &results](TaskContext& tc, uint32_t index)
			{
				const auto crcStart = (index * crcPageSize);
				const auto crcEnd = std::min<uint64_t>(actualReadSize, crcStart + crcPageSize);

				auto crc = CRC64().append(memory + crcStart, crcEnd - crcStart).crc();

				auto absoluteOffset = readStart + crcStart;
				results.lock()->emplaceBack(std::make_pair(absoluteOffset, crc));
			};

			sig.registerCompletionCallback([memory, &memoryPool, readSize]()
				{
					memoryPool.free(memory, readSize);
				});

			pendingSignals.lock()->pushBack(sig);
		}
		else
		{
			TRACE_WARNING("Failed async at {}-{}", readStart, readEnd);
			memoryPool.free(memory, readSize);
		}
	};

	while (!sig.waitSpinWithTimeout(33))
	{
		MarkProfilingFrame();
	}

	auto finalSig = TaskSignal::Merge(*pendingSignals.lock());

	{
		ScopeTimer finalWait;
		while (!finalSig.waitSpinWithTimeout(33))
		{
			MarkProfilingFrame();
		}
		TRACE_WARNING("Final wait time: {}", finalWait);
	}

	results.lock()->sort([](auto a, auto b)
		{
			return a.first < b.first;
		});

	CRC64 finalResult;
	for (const auto p : *results.lock())
		finalResult << p.second;

	TRACE_WARNING("Final result: {} after {}", Hex(finalResult.crc()), timer);
}
#endif
#else

//--

TEST(AsyncFile, BigReadWithMemoryMapTasks)
{
	ScopeTimer timer;

	auto file = FileSystem().openForReading("Z:/unity_source_code.rar", FileReadMode::MemoryMapped);
	ASSERT_TRUE(!!file);

	auto size = file->size();
	TRACE_INFO("File size: {}", MemSize(size));

	auto view = file->createMapping(file->fullRange());
	ASSERT_TRUE(!!view);

	const auto pageSize = 1ULL << 20;
	const auto crcPageSize = pageSize / 10;
	const auto crcPageCount = (size + crcPageSize - 1) / crcPageSize;

	ThreadSafe<Array<std::pair<uint64_t, uint64_t>>> results;

	auto sig = TaskBuilder("CRCTest"_id).instances(crcPageCount) << [&view, crcPageSize, size, &results](TaskContext& tc, uint32_t index)
	{
		const auto crcStart = (index * crcPageSize);
		const auto crcEnd = std::min<uint64_t>(size, crcStart + crcPageSize);

		auto crc = CRC64().append(view->data() + crcStart, crcEnd - crcStart).crc();
		results.lock()->emplaceBack(std::make_pair(crcStart, crc));
	};

	while (!sig.waitSpinWithTimeout(33))
	{
		MarkProfilingFrame();
	}

	results.lock()->sort([](auto a, auto b)
		{
			return a.first < b.first;
		});

	CRC64 finalResult;
	for (const auto p : *results.lock())
		finalResult << p.second;

	TRACE_WARNING("Final result: {} after {}", Hex(finalResult.crc()), timer);
}
#endif

#endif

END_INFERNO_NAMESPACE()