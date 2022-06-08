/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "implSystemVirtualMemory.h"
#include "bm/core/system/include/algorithms.h"

//#define ALLOCATE_PAGES_FROM_HEAP
//#define FORCE_FREE_PAGES

#ifndef ALLOCATE_PAGES_FROM_HEAP
#if defined(PLATFORM_POSIX)
#include <sys/mman.h>
#elif defined(PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(PLATFORM_PSX)
#include <kernel.h>
#endif
#endif

BEGIN_INFERNO_NAMESPACE()

//--

PoolPaged_SystemVirtualMemory::PoolPaged_SystemVirtualMemory(const char* name, const PoolPaged_SystemVirtualMemorySetup& setup)
	: IPoolPaged(name)
	, m_setup(setup)
	, m_cachePageCount(0)
	, m_cachePageSize(0)
{
	ASSERT(setup.minimumPageSize <= setup.maximumPageSize);

	m_allocatedPages.reserve(1024);
	m_minSizeLog2 = FloorLog2(setup.minimumPageSize);
	m_maxSizeLog2 = FloorLog2(NextPowerOf2(setup.maximumPageSize));

	for (auto& bucket : m_freePagesBuckets)
		bucket.pages.reserve(1024);
}

PoolPaged_SystemVirtualMemory::~PoolPaged_SystemVirtualMemory()
{
}

uint64_t PoolPaged_SystemVirtualMemory::pageSizeForBucket(uint8_t bucket) const
{
	return 1ULL << (m_minSizeLog2 + bucket);
}

//--

PoolPaged_SystemVirtualMemory::PageInfo PoolPaged_SystemVirtualMemory::allocateRawPage(uint64_t size)
{
	ASSERT(IsPowerOf2(size));
	const auto bucketIndex = FloorLog2(size);

	// try the cache
	{
		auto& freeBucket = m_freePagesBuckets[bucketIndex];

		// lock only the bucket for appropriate size
		auto lock = CreateLock(freeBucket.lock);
		if (!freeBucket.pages.empty())
		{
			auto page = freeBucket.pages.back();
			ASSERT(page.size == size);
			ASSERT(page.bucket == bucketIndex);
			freeBucket.pages.pop_back();

			m_cachePageCount -= 1;
			m_cachePageSize -= page.size;

			return page;
		}
	}

	// allocate new page
	PageInfo page;
	page.bucket = bucketIndex;
	page.basePtr = allocateVirtualMemory(size, page.size);
	DEBUG_CHECK_RETURN_EX_V(page.basePtr, TempString("Out of memory allocating memory page {}", MemSize(size)), PageInfo());

	// update stats
	notifyAllocation(page.size);
	return page;
}

void PoolPaged_SystemVirtualMemory::freeRawPage(PageInfo page)
{
	ASSERT(page.basePtr != nullptr);
	ASSERT(page.bucket != -1);

	// free memory directly
#ifdef FORCE_FREE_PAGES
	const bool shouldFree = page.size > 0;
#else
	const bool shouldFree = page.size == 0;
#endif
	if (shouldFree)
	{
		freeVirtualMemory(page.basePtr, page.size);
	}
	else
	{
		// return page to bucket
		auto& freeBucket = m_freePagesBuckets[page.bucket];

		{
			auto lock = CreateLock(freeBucket.lock);
			freeBucket.pages.push_back(page);
		}

		// update cache stats
		m_cachePageCount += 1;
		m_cachePageSize += page.size;
	}

	// update allocation stats
	notifyFree(page.size);
}

uint32_t PoolPaged_SystemVirtualMemory::registerRawPage(const PageInfo& info)
{
	auto index = m_nextPageIndex++;

	{
		auto lock = CreateLock(m_allocatedPagesLock);
		m_allocatedPages[index] = info;
	}

	return index;
}

PoolPaged_SystemVirtualMemory::PageInfo PoolPaged_SystemVirtualMemory::unregisterRawPage(MemoryPage page)
{
	auto lock = CreateLock(m_allocatedPagesLock);
	auto it = m_allocatedPages.find(page.index);
	DEBUG_CHECK_RETURN_V(it != m_allocatedPages.end(), PageInfo());

	auto pageInfo = it->second;
	m_allocatedPages.erase(it);

	ASSERT_EX((uint8_t*)pageInfo.basePtr == page.basePtr, "Invalid page");

	return pageInfo;
}

MemoryPage PoolPaged_SystemVirtualMemory::allocatPage(uint64_t size)
{
	DEBUG_CHECK_RETURN_EX_V(size, "Invalid page size", MemoryPage());
	DEBUG_CHECK_RETURN_EX_V(IsPowerOf2(size), "Page size must be power of two", MemoryPage());

	const auto rawPage = allocateRawPage(size);
	VALIDATION_RETURN_V(rawPage.basePtr, MemoryPage());
	ASSERT(rawPage.size >= size);

	MemoryPage page;
	page.index = registerRawPage(rawPage);
	page.basePtr = (uint8_t*)rawPage.basePtr;
	page.endPtr = page.basePtr + rawPage.size;

	return page;
}

void PoolPaged_SystemVirtualMemory::freePage(MemoryPage page)
{
	auto rawPage = unregisterRawPage(page);
	DEBUG_CHECK_RETURN_EX(rawPage.basePtr, "Invalid page to free");

	freeRawPage(rawPage);
}

uint64_t PoolPaged_SystemVirtualMemory::queryMinimumPageSize() const
{
	return m_setup.minimumPageSize;
}

uint64_t PoolPaged_SystemVirtualMemory::queryMaximumPageSize() const
{
	return m_setup.maximumPageSize;
}

void PoolPaged_SystemVirtualMemory::print(IFormatStream& f, int details /*= 0*/) const
{
	IPoolPaged::print(f, details);

	f.appendf(" Cached: {} ({} pages)\n", MemSize(m_cachePageSize), m_cachePageCount);
}

//---

#ifndef ALLOCATE_PAGES_FROM_HEAP
#ifdef PLATFORM_WINDOWS
static size_t GetPageSize()
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	return systemInfo.dwPageSize;
}
#endif
#endif

void* PoolPaged_SystemVirtualMemory::allocateVirtualMemory(uint64_t size, uint64_t& outAllocatedSize) const
{
	void* ret = nullptr;

#ifdef ALLOCATE_PAGES_FROM_HEAP
	const auto allocSize = Align<uint64_t>(size, 4096);
	outAllocatedSize = allocSize;
	ret = malloc(allocSize);
#else
#ifdef PLATFORM_WINAPI
	static auto largePageSize = GetLargePageMinimum();
	static auto smallPageSize = GetPageSize();
	const auto useLargePages = size >= largePageSize;
	const auto pageSize = useLargePages ? largePageSize : smallPageSize;
	const auto allocSize = Align<uint64_t>(size, pageSize);

	DWORD flags = 0;
	if (m_setup.flagCpuExecutable)
	{
		if (m_setup.flagCpuWritable)
			flags |= PAGE_EXECUTE_READWRITE;
		else
			flags |= PAGE_EXECUTE_READ;
	}
	else
	{
		if (m_setup.flagCpuWritable)
			flags |= PAGE_READWRITE;
		else
			flags |= PAGE_READONLY;
	}

	outAllocatedSize = allocSize;
	ret = VirtualAlloc(NULL, allocSize, MEM_COMMIT | MEM_RESERVE | (useLargePages ? MEM_LARGE_PAGES : 0), flags);

	if (useLargePages && ret == nullptr)
	{
		auto newAllocSize = Align<uint64_t>(size, smallPageSize);
		outAllocatedSize = newAllocSize;
		ret = VirtualAlloc(NULL, newAllocSize, MEM_COMMIT | MEM_RESERVE, flags);
	}
#elif defined(PLATFORM_POSIX)
	DWORD flags = 0;
	if (m_setup.flagCpuExecutable)
		flags |= PROT_EXECUTE;
	if (m_setup.flagCpuReadable)
		flags |= PROT_READ;
	if (m_setup.flagCpuWritable)
		flags |= PROT_WRITE;

	static auto pageSize = 4096;
	auto allocSize = Align<uint64_t>(size, pageSize);
	outAllocatedSize = allocSize;
	ret = mmap64(nullptr, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#elif defined(PLATFORM_PSX)
	const auto alignment = 1U << 16; // 64K

	// Memory type
	auto memoryType = SCE_KERNEL_MTYPE_C;
	if (!m_setup.flagGpuReadable && !m_setup.flagGpuWritable)
		memoryType = SCE_KERNEL_MTYPE_C_SHARED;

	// First allocate the physical memory
	off_t directAddress = 0;
	auto result = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, size, alignment, memoryType, &directAddress);

	// allocated, map it
	if (result == SCE_OK)
	{
		uint32_t memProtection = 0;
		if (m_setup.flagCpuReadable)
			memProtection |= SCE_KERNEL_PROT_CPU_READ;
		if (m_setup.flagCpuWritable)
			memProtection |= SCE_KERNEL_PROT_CPU_WRITE;
		if (m_setup.flagCpuExecutable)
			memProtection |= SCE_KERNEL_PROT_CPU_EXEC;
		if (m_setup.flagGpuReadable)
			memProtection |= SCE_KERNEL_PROT_GPU_READ;
		if (m_setup.flagGpuWritable)
			memProtection |= SCE_KERNEL_PROT_GPU_WRITE;

		void* virtualAddress = nullptr;
		outAllocatedSize = size;
		ret = sceKernelMapDirectMemory2(&virtualAddress, size, memoryType, memProtection, SCE_KERNEL_MAP_NO_OVERWRITE, directAddress, alignment);
		if (result != SCE_OK)
		{
			sceKernelReleaseDirectMemory(directAddress, size);
		}
	}
#endif
#endif

	ASSERT_EX(ret != nullptr, "Out of virtual memory");
	return ret;
}

void PoolPaged_SystemVirtualMemory::freeVirtualMemory(void* ptr, uint64_t size) const
{
#ifdef ALLOCATE_PAGES_FROM_HEAP
	free(ptr);
#else
#ifdef PLATFORM_WINAPI
	VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(PLATFORM_POSIX)
	munmap(page, size);
#elif defined(PLATFORM_PSX)
	SceKernelVirtualQueryInfo virtualMemoryInfo;
	memzero(&virtualMemoryInfo, sizeof(virtualMemoryInfo));

	auto result = ::sceKernelVirtualQuery(ptr, 0, &virtualMemoryInfo, sizeof(virtualMemoryInfo));
	if (result == SCE_OK)
	{
		::sceKernelMunmap(ptr, size);

		if (virtualMemoryInfo.isDirectMemory)
			::sceKernelReleaseDirectMemory(virtualMemoryInfo.offset, size);
		else if (virtualMemoryInfo.isFlexibleMemory)
			::sceKernelReleaseFlexibleMemory(ptr, size);
	}
#else
	#error "Unknown platform"
#endif
#endif
}

//---

// TODO: find better way of initializing this

static IPoolPaged* GLocalPagePool = new PoolPaged_SystemVirtualMemory("LocalPagePool", PoolPaged_SystemVirtualMemorySetup());

IPoolPaged& LocalPagePool()
{
	return *GLocalPagePool;
}

//---

END_INFERNO_NAMESPACE()
