/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// parallel for helper with calling thread participation
class BM_CORE_TASK_API TaskParallelFor : public NoCopy
{
public:
	TaskParallelFor(IndexRange range);
	TaskParallelFor(TaskContext& tc, IndexRange range);

	TaskParallelFor& block(uint32_t size); // minimal work unit size (number of elements)
	TaskParallelFor& concurency(uint32_t concurency); // desired level of concurency

	TaskParallelFor& operator<<(TTaskForFunc func); // starts processing
	TaskParallelFor& operator<<(TTaskForFuncEx func); // starts processing

	static void CalculateWorkloads(IndexRange range, uint32_t blockSize, uint32_t concurency, IndexRange& outMainWorkload, IndexRange& outTaskWorkloads, uint32_t& outTaskBlockSize);
	
private:
	IndexRange m_range;
	TaskContext& m_task;

	uint32_t m_blockSize = 1;
	uint32_t m_concurency = std::numeric_limits<uint32_t>::max();
};

//--

/// parallel for helper with calling thread participation
class BM_CORE_TASK_API TaskParallelForEach : public NoCopy
{
public:
	TaskParallelForEach(IndexRange range);
	TaskParallelForEach(TaskContext& tc, IndexRange range);

	TaskParallelForEach& concurency(uint32_t concurency); // desired level of concurency

	TaskParallelForEach& operator<<(TTaskInstancingFunc func); // starts processing
	TaskParallelForEach& operator<<(TTaskInstancingFuncEx func); // starts processing

	static void CalculateWorkloads(IndexRange range, uint32_t concurency, IndexRange& outMainWorkload, IndexRange& outTaskWorkload);

private:
	IndexRange m_range;
	TaskContext& m_task;

	uint32_t m_concurency = std::numeric_limits<uint32_t>::max();
};

//--

/// parallel for helper with calling thread participation
template< typename T >
class TaskParallelForT : public NoCopy
{
public:
	INLINE TaskParallelForT(ArrayView<T> ar)
		: m_for(ar.indexRange())
		, m_array(ar)
	{}

	INLINE TaskParallelForT(TaskContext& tc, ArrayView<T> ar)
		: m_for(tc, ar.indexRange())
		, m_array(ar)
	{}

	INLINE TaskParallelForT& block(uint32_t size)
	{
		m_for.block(size);
		return *this;
	}

	INLINE TaskParallelForT& concurency(uint32_t concurency)
	{
		m_for.concurency(concurency);
		return *this;
	}

	INLINE TaskParallelForT& operator<<(std::function<void(ArrayView<T>)> func)
	{
		m_for << [&func, this](IndexRange range) { func(m_array[range]); };
		return *this;
	}

	INLINE TaskParallelForT& operator<<(std::function<void(TaskContext& tc, ArrayView<T>)> func)
	{
		m_for << [&func, this](TaskContext& tc, IndexRange range) { func(tc, m_array[range]); };
		return *this;
	}

private:
	TaskParallelFor m_for;
	ArrayView<T> m_array;
};

//--

/// parallel for helper with calling thread participation
template< typename T >
class TaskParallelForEachT : public NoCopy
{
public:
	INLINE TaskParallelForEachT(ArrayView<T> ar)
		: m_forEach(ar.indexRange())
		, m_array(ar)
	{
	}

	INLINE TaskParallelForEachT(TaskContext& tc, ArrayView<T> ar)
		: m_forEach(tc, ar.indexRange())
		, m_array(ar)
	{
	}

	INLINE TaskParallelForEachT& concurency(uint32_t concurency)
	{
		m_forEach.concurency(concurency);
		return *this;
	}

	INLINE TaskParallelForEachT& operator<<(std::function<void(T)> func)
	{
		m_forEach << [&func, this](uint32_t index) { func(m_array[index]); };
		return *this;
	}

	INLINE TaskParallelForEachT& operator<<(std::function<void(TaskContext& tc, T)> func)
	{
		m_forEach << [&func, this](TaskContext& ctx, uint32_t index) { func(ctx, m_array[index]); };
		return *this;
	}

private:
	TaskParallelForEach m_forEach;
	ArrayView<T> m_array;
};

//--

END_INFERNO_NAMESPACE()

