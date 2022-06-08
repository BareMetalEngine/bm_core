/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskSignal.h"
#include "taskUtils.h"
#include "bm/core/containers/include/inplaceArray.h"
#include "taskBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//--

TaskParallelFor::TaskParallelFor(IndexRange range)
	: m_range(range)
	, m_task(NoTask())
{}

TaskParallelFor::TaskParallelFor(TaskContext& tc, IndexRange range)
	: m_range(range)
	, m_task(tc)
{}

TaskParallelFor& TaskParallelFor::block(uint32_t size)
{
	DEBUG_CHECK_RETURN_EX_V(size >= 1, "Block size can't be zero", *this);
	m_blockSize = size;
	return *this;
}

TaskParallelFor& TaskParallelFor::concurency(uint32_t size)
{
	DEBUG_CHECK_RETURN_EX_V(size >= 1, "Concurency should be at least 1", *this);
	m_concurency = size;
	return *this;
}

TaskParallelFor& TaskParallelFor::operator<<(TTaskForFunc func)
{
	PC_SCOPE_LVL1(ParallelFor);

	uint32_t taskBlockSize = 0;
	IndexRange mainRange, taskRanges;
	CalculateWorkloads(m_range, m_blockSize, m_concurency, mainRange, taskRanges, taskBlockSize);

	if (!taskRanges.empty())
	{
		const auto blockCount = taskRanges.blockCount(taskBlockSize);
		auto sig = TaskBuilder(m_task, "ParallelFor"_id).instances(blockCount).concurency(m_concurency) << [&taskRanges, taskBlockSize, &func](uint32_t index)
		{
			const auto indexRange = taskRanges.blockRange(taskBlockSize, index);
			func(indexRange);
		};

		if (mainRange)
		{
			PC_SCOPE_LVL1(Main);
			func(mainRange);
		}
	
		{
			PC_SCOPE_LVL1(WaitForFinish);
			sig.waitSpinInfinite();
		}
	}
	else if (mainRange)
	{
		PC_SCOPE_LVL1(Main);
		func(mainRange);
	}

	return *this;
}

TaskParallelFor& TaskParallelFor::operator<<(TTaskForFuncEx func)
{
	PC_SCOPE_LVL1(ParallelFor);

	uint32_t taskBlockSize=0;
	IndexRange mainRange, taskRanges;
	CalculateWorkloads(m_range, m_blockSize, m_concurency, mainRange, taskRanges, taskBlockSize);

	if (!taskRanges.empty())
	{
		const auto blockCount = taskRanges.blockCount(taskBlockSize);
		auto sig = TaskBuilder(m_task, "ParallelFor"_id).instances(blockCount).concurency(m_concurency) << [&taskRanges, taskBlockSize, &func](TaskContext& tc, uint32_t index)
		{
			const auto indexRange = taskRanges.blockRange(taskBlockSize, index);
			func(tc, indexRange);
		};

		if (mainRange)
		{
			PC_SCOPE_LVL1(Main);
			func(m_task, mainRange);
		}

		{
			PC_SCOPE_LVL1(WaitForFinish);
			sig.waitSpinInfinite();
		}
	}
	else if (mainRange)
	{
		PC_SCOPE_LVL1(Main);
		func(m_task, mainRange);
	}

	return *this;
}

//--

void TaskParallelFor::CalculateWorkloads(IndexRange range, uint32_t blockSize, uint32_t concurency, IndexRange& outMainWorkload, IndexRange& outTaskWorkloads, uint32_t& outTaskBlockSize)
{
	outTaskWorkloads = IndexRange();
	outTaskBlockSize = 0;

	concurency = std::min<uint32_t>(concurency, MaxTaskConcurency());

	if (range.size() <= blockSize || concurency == 1)
	{
		outMainWorkload = range;
	}
	else
	{
		const auto numBlocks = range.blockCount(blockSize);
		const auto numBlocksPerWorker = numBlocks / concurency;
		const auto numRemainingBlocks = numBlocks - (numBlocksPerWorker * concurency);

		const auto numMainBlocks = numBlocksPerWorker + (numRemainingBlocks ? 1 : 0);
		const auto numMainElements = numMainBlocks * blockSize;
		outMainWorkload = IndexRange(range.first(), numMainElements);

		// if anything is left distribute among tasks
		if (numMainElements < range.size())
		{
			outTaskBlockSize = blockSize; // TODO: consider tuning this to lower value than max to aid with equal utilization
			outTaskWorkloads = IndexRange(range.first() + numMainElements, range.size() - numMainElements);

			// ensure average workload per core is not larger than main workload
			const auto averageWorkloadPerWoker = outTaskWorkloads.size() / concurency;
			ASSERT(averageWorkloadPerWoker <= outMainWorkload.size());
		}
	}
}

//--

TaskParallelForEach::TaskParallelForEach(IndexRange range)
	: m_range(range)
	, m_task(NoTask())
{}

TaskParallelForEach::TaskParallelForEach(TaskContext& tc, IndexRange range)
	: m_range(range)
	, m_task(tc)
{}

TaskParallelForEach& TaskParallelForEach::concurency(uint32_t size)
{
	DEBUG_CHECK_RETURN_EX_V(size >= 1, "Concurency should be at least 1", *this);
	m_concurency = size;
	return *this;
}

TaskParallelForEach& TaskParallelForEach::operator<<(TTaskInstancingFunc func)
{
	PC_SCOPE_LVL1(ParallelForEach);

	IndexRange mainRange, taskRanges;
	CalculateWorkloads(m_range, m_concurency, mainRange, taskRanges);

	if (!taskRanges.empty())
	{
		const auto first = taskRanges.first();

		auto sig = TaskBuilder(m_task, "ParallelForEach"_id).instances(taskRanges.size()).concurency(m_concurency) << [&func, first](TaskContext& tc, uint32_t index)
		{
			func(tc, index + first);
		};

		if (mainRange)
		{
			PC_SCOPE_LVL1(Main);
			for (auto index : mainRange)
				func(m_task, index);
		}

		{
			PC_SCOPE_LVL1(WaitForFinish);
			sig.waitSpinInfinite();
		}
	}
	else if (mainRange)
	{
		PC_SCOPE_LVL1(Main);
		for (auto index : mainRange)
			func(m_task, index);
	}

	return *this;
}

TaskParallelForEach& TaskParallelForEach::operator<<(TTaskInstancingFuncEx func)
{
	PC_SCOPE_LVL1(ParallelForEach);

	IndexRange mainRange, taskRanges;
	CalculateWorkloads(m_range, m_concurency, mainRange, taskRanges);

	if (!taskRanges.empty())
	{
		const auto first = taskRanges.first();

		auto sig = TaskBuilder(m_task, "ParallelForEach"_id).instances(taskRanges.size()).concurency(m_concurency) << [&func, first](TaskContext& tc, uint32_t index)
		{
			func(index + first);
		};

		if (mainRange)
		{
			PC_SCOPE_LVL1(Main);
			for (auto index : mainRange)
				func(index);
		}

		{
			PC_SCOPE_LVL1(WaitForFinish);
			sig.waitSpinInfinite();
		}
	}
	else if (mainRange)
	{
		PC_SCOPE_LVL1(Main);
		for (auto index : mainRange)
			func(index);
	}

	return *this;
}

//--

void TaskParallelForEach::CalculateWorkloads(IndexRange range, uint32_t concurency, IndexRange& outMainWorkload, IndexRange& outTaskWorkload)
{
	concurency = std::min<uint32_t>(concurency, MaxTaskConcurency());

	if (range.size() <= 1 || concurency == 1)
	{
		outTaskWorkload = IndexRange();
		outMainWorkload = range;
	}
	else
	{
		const auto numElementsPerWorker = range.size() / concurency;
		const auto numRemainingElements = range.size() - (numElementsPerWorker * concurency);

		const auto numMainElements = numElementsPerWorker + (numRemainingElements ? 1 : 0);
		outMainWorkload = IndexRange(range.first(), numMainElements);
		outTaskWorkload = IndexRange(range.first() + numMainElements, range.size() - numMainElements);
	}
}

//--

END_INFERNO_NAMESPACE()
