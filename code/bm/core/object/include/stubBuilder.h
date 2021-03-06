/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: stubs #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

class StubFactory;

// linear allocator based factory, used when building the stubs
class BM_CORE_OBJECT_API StubBuilder : public MainPoolData<NoCopy>
{
public:
	StubBuilder(LocalAllocator& mem, const StubFactory& factory);
	~StubBuilder();

	// remove all created stubs
	void clear();

	//--

	void* createData(uint32_t size);

	template< typename T >
	INLINE T* createStub()
	{
		return static_cast<T*>(createStub(T::StaticType()));
	}

	template< typename T >
	INLINE StubPseudoArray<T> createArray(const Array<const T*>& stubs)
	{
		StubPseudoArray<T> ret;

		if (!stubs.empty())
		{
			ret.elemCount = stubs.size();
			ret.elems = (const T* const*)allocateInternal(sizeof(IStub*) * stubs.size());
			memcpy((void*)ret.elems, stubs.data(), stubs.dataSize());
		}

		return ret;
	}

	template< typename T >
	INLINE StubPseudoArray<T> createArray(const Array<T*>& stubs)
	{
		StubPseudoArray<T> ret;
		ret.elemCount = stubs.size();
		ret.elems = (const T* const*)allocateInternal(sizeof(IStub*) * stubs.size());
		memcpy((void*)ret.elems, stubs.data(), stubs.dataSize());
		return ret;
	}

protected:
	LocalAllocator& m_mem;
	const StubFactory& m_factory;

	Array<IStub*> m_stubsToDestroy;

	IStub* createStub(StubTypeValue id);
	void* allocateInternal(uint32_t size);
};

//---

END_INFERNO_NAMESPACE()
