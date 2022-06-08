/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

// Glue headers and logic
#include "bm_core_memory_glue.inl"

//-----------------------------------------------------------------------------

// This needs to be declared so we do not redefine new/delete operators
#define __PLACEMENT_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE

//-----------------------------------------------------------------------------

BEGIN_INFERNO_NAMESPACE()


//--

class IPool;
class IPoolPaged;
class IPoolUnmanaged;
class IPoolManaged;

class LocalAllocator;

//--

// general engine-wide compression types
enum class CompressionType : uint8_t
{
	Uncompressed = 0, 	// no compression, the compression API will work but probably it's not the best use of the API :)
	Zlib,	
	LZ4,	
	LZ4HC,

	MAX, // keep last
};

// general engine-wide encoding types
enum class EncodingType : uint8_t
{
	Base64 = 0, // Standard base64 encoding (constant size, some effort to decode)
	Hex = 1, // Hexadecimal encoding (cheap to decode)
	URL = 2, // URL encoding (non const size)
	CString = 3, // C string encoding (non const size)
};

//--

struct MemoryPage
{
	uint32_t index = 0; // internal index
	uint8_t* basePtr = nullptr;
	uint8_t* endPtr = nullptr;

	INLINE uint64_t size() const { return endPtr - basePtr; }
};

//--

//! Get main memory pool (application global)
extern BM_CORE_MEMORY_API IPoolUnmanaged& MainPool();

//! Get extended memory pool, all non mission critical stuff should be allocated from it (especially all stuff related to debug/editor code)
extern BM_CORE_MEMORY_API IPoolUnmanaged& ExtendedPool();

//! Special pool for large memory allocation
extern BM_CORE_MEMORY_API IPoolUnmanaged& LargeAllocationPool();

//! Get pool for local page allocations (allocation done via LocalAllocator that are meant to be short lived within frame)
//! NOTE: this allocator is meant to store rather big pages, not single allocations
extern BM_CORE_MEMORY_API IPoolPaged& LocalPagePool();

///--

//! Proxy functions for memory allocation from unmanaged pool so we don't have to include header
extern BM_CORE_MEMORY_API void* PoolAllocate(IPoolUnmanaged& pool, uint64_t size, uint32_t alignment);

//! Proxy functions for memory reallocation from unmanaged pool so we don't have to include header
extern BM_CORE_MEMORY_API void* PoolReallocate(IPoolUnmanaged& pool, void* ptr, uint64_t size, uint32_t alignment);

//! Proxy functions for memory release from unmanaged pool so we don't have to include header
extern BM_CORE_MEMORY_API void PoolFree(IPoolUnmanaged& pool, void* ptr);

///--

//! Construct new object from memory from pool
template< typename T, typename ...Args >
ALWAYS_INLINE T* PoolNew(IPoolUnmanaged& pool, Args&&... args)
{
	return new (PoolAllocate(pool, sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
}

//! Delete object constructed from pool's memory
template< typename T >
ALWAYS_INLINE void PoolDelete(IPoolUnmanaged& pool, T* ptr)
{
	ptr->~T();
	PoolFree(pool, ptr);
}

///--

#define DECLARE_POOL_ALLOCATOR(_poolName, _base) \
    public: \
	ALWAYS_INLINE static void* AllocateClassMemory(std::size_t size, uint32_t align) { return PoolAllocate(_poolName(), size, align);} \
	ALWAYS_INLINE static void FreeClassMemory(void* ptr) { PoolFree(_poolName(), ptr); } \
    private:\

#define DECLARE_POOL_WRAPPERS_FREE(_base) \
    public: \
    ALWAYS_INLINE void operator delete(void* ptr, std::size_t sz) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete[](void* ptr, std::size_t sz) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete(void* ptr, std::size_t sz, std::align_val_t al) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete[](void* ptr, std::size_t sz, std::align_val_t al) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete(void* ptr) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete(void* ptr, std::align_val_t al) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete[](void* ptr) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete[](void* ptr, std::align_val_t al) { FreeClassMemory(ptr); } \
    ALWAYS_INLINE void operator delete(void *pt, void* pt2) {} \
    private:\

#define DECLARE_POOL_WRAPPERS_NEW_NO_PATCH(_base) \
    public: \
    ALWAYS_INLINE void* operator new(std::size_t count) { return AllocateClassMemory(count, (uint32_t)alignof(_base)); } \
    ALWAYS_INLINE void* operator new[](std::size_t count) { return AllocateClassMemory(count, (uint32_t)alignof(_base)); } \
    ALWAYS_INLINE void* operator new(std::size_t count, std::align_val_t al) { return AllocateClassMemory(count, (uint32_t)al); } \
    ALWAYS_INLINE void* operator new[](std::size_t count, std::align_val_t al) { return AllocateClassMemory(count, (uint32_t)al); } \
    ALWAYS_INLINE void* operator new(std::size_t count, void* pt) { return pt; } \
    private:\

#define DECLARE_POOL_WRAPPERS_NEW_PATCH(_base) \
    public: \
    ALWAYS_INLINE void* operator new(std::size_t count) { return IObject::PatchClassPointer(AllocateClassMemory(count, (uint32_t)alignof(_base)), _base::GetStaticClass().ptr()); } \
    ALWAYS_INLINE void* operator new[](std::size_t count) { return IObject::PatchClassPointer(AllocateClassMemory(count, (uint32_t)alignof(_base)), _base::GetStaticClass().ptr()); } \
	ALWAYS_INLINE void* operator new(std::size_t count, std::align_val_t al) { return IObject::PatchClassPointer(AllocateClassMemory(count, (uint32_t)al), _base::GetStaticClass().ptr()); } \
    ALWAYS_INLINE void* operator new[](std::size_t count, std::align_val_t al) {  return IObject::PatchClassPointer(AllocateClassMemory(count, (uint32_t)al), _base::GetStaticClass().ptr()); } \
    ALWAYS_INLINE void* operator new(std::size_t count, void* pt) { return IObject::PatchClassPointer(pt, _base::GetStaticClass().ptr()); } \
    private:\

#define DECLARE_POOL_BASE(_poolName, _base) \
	DECLARE_POOL_ALLOCATOR(_poolName, _base) \
	DECLARE_POOL_WRAPPERS_FREE(_base) \
	DECLARE_POOL_WRAPPERS_NEW_NO_PATCH(_base)

template< typename T >
struct MainPoolData : public T
{
	DECLARE_POOL_BASE(MainPool, T);
};

template< typename T >
struct ExtendedPoolData : public T
{
	DECLARE_POOL_BASE(ExtendedPool, T);
};

//--

// DEPRECATED memory "manager"
class Memory
{
public:
	//--

	static void* AllocSystemMemory(size_t size, bool largePages, const char* tag)
	{
		return PoolAllocate(MainPool(), size, 16);
	}

	static void FreeSystemMemory(void* page, size_t size)
	{
		PoolFree(MainPool(), page);
	}

	static void* AllocateBlock(size_t size, size_t alignment, const char* tag)
	{
		return PoolAllocate(MainPool(), size, alignment);
	}

	static void* ResizeBlock(void* mem, size_t size, size_t alignment, const char* tag)
	{
		return PoolReallocate(MainPool(), mem, size, alignment);
	}

	static void FreeBlock(void* mem)
	{
		PoolFree(MainPool(), mem);
	}

};

static const uint32_t DEFAULT_BUFFER_ALIGNMENT = 16;

END_INFERNO_NAMESPACE()

//--
