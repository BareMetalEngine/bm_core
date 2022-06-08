/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/stackAllocator.h"
#include "bm/core/memory/include/structureAllocator.h"

BEGIN_INFERNO_NAMESPACE()

///--

// data for the low-level memory block
struct MemoryBlockData : public MainPoolData<NoCopy>
{
    uint8_t allocated : 1;
    uint8_t locked : 1;
    uint64_t address;
    uint64_t size;
	MemoryBlockData* next;
	MemoryBlockData* prev;

	INLINE MemoryBlockData() {};
};

///--

class MemoryBlockPool;

struct BM_CORE_CONTAINERS_API MemoryBlock
{
public:
    INLINE MemoryBlock() = default;
    INLINE MemoryBlock(std::nullptr_t) {};
    INLINE MemoryBlock(const MemoryBlock& other) = default;
    INLINE MemoryBlock& operator=(const MemoryBlock& other) = default;

    INLINE operator bool() const { return m_block != nullptr; }

    INLINE uint64_t address() const { return m_block->address; }
    INLINE uint64_t size() const { return m_block->size; }

    void print(IFormatStream& f) const;

private:
    const MemoryBlockData* m_block = nullptr;

    friend class MemoryBlockPool;
};

///--

/// defragmentation operation request
struct MemoryBlockDefragRegion
{
	uint64_t sourceAddress = 0;
	uint64_t destinationAddress = 0;
	uint32_t size = 0;
};

/// memory statistics
struct MemoryBlockPoolStats
{
	uint64_t totalPoolSize = 0; // size of the whole pool
	uint32_t totalBlockCount = 0; // total number of blocks this pool can service

	uint64_t currentAllocatedBytes = 0; // number of bytes (with alignment) currently allocated from the pool
	uint64_t maxAllocatedBytes = 0; // maximum amount of bytes ever allocated from the pool
	uint32_t currentAllocatedBlocks = 0; // number of blocks currently allocated from the pool
	uint32_t maxAllocatedBlocks = 0; // maximum amount of blocks ever allocated from the pool
	uint64_t wastedBytes = 0; // bytes that were wasted for alignment and small blocks

	uint64_t largestFreeBlockBytes = 0; // size of the largest free block this pool
	uint32_t freeBlockCount = 0; // number of free blocks
};

/// block memory allocator with support for defragmentation
/// NOTE: this is used only by drivers that can manage their own memory
class BM_CORE_CONTAINERS_API MemoryBlockPool : public MainPoolData<NoCopy>
{
public:
	MemoryBlockPool();
	~MemoryBlockPool();
		
	///---

	/// get base address of the pool
	INLINE uint64_t baseAddress() const { return m_baseAddress; }

    /// get total size of the pool
    INLINE uint64_t size() const { return m_totalSize; }

	/// number of allocated bytes
	INLINE uint64_t numAllocatedBytes() const { return m_numAllocatedBytes; };

    /// number of allocated blocks
    INLINE uint32_t numAllocatedBlocks() const { return m_numAllocatedBlocks; }

	///---

	/// initialize allocator with given size and number of block, resets current content
	void reset(uint64_t baseAddress, uint64_t size, uint32_t minFreeBlockSize);

	/// get current statistics
	void stats(MemoryBlockPoolStats& outStats) const;

	///---

	/// allocate block in the allocator
	MemoryBlock allocateBlock(uint32_t size, uint32_t alignment);

	/// unlock block that was locked
	void unlockBlock(MemoryBlock block);

	/// free previously allocated block
	void freeBlock(MemoryBlock block);

	///---

	/// defragmentation interface - get list of blocks to copy
	/// the returned blocks are internally marked as "LOCKED" for the duration of the move
	/// the request can be throttled by specifying the maximum amount of data and/or blocks that we want to move 
	/// NOTE: we MUST call the signalBlockMoved for EACH block returned here or else we will leak the memory permanently
	/// NOTE: the calls can overlap (you may call requestBlocksToMove again before calling signalBlockMoved on all previously returned blocks)
	void collectBlocksForDefrag(uint64_t maxBytesToMove, uint32_t maxBlocksToMove, Array<MemoryBlockDefragRegion>& outBlocks);

	/// signal that the blocks previously returned by requestBlocksToMove were all moved
	void finishBlockDefrag();

	///---

private:
	INLINE static void UnlinkBlock(MemoryBlockData* ptr)
	{
		if (ptr->next)
			ptr->next->prev = ptr->prev;
		if (ptr->prev)
			ptr->prev->next = ptr->next;
		ptr->next = nullptr;
		ptr->prev = nullptr;
	}

	INLINE static void LinkBlock(MemoryBlockData* ptr, MemoryBlockData*& head)
	{
        ptr->prev = nullptr;
		ptr->next = head;
		if (head)
			head->prev = ptr;
		head = ptr;
	}

	INLINE static void LinkBlockAfter(MemoryBlockData* ptr, MemoryBlockData* after)
	{
        ptr->prev = after;
		ptr->next = after->next;

		if (after->next)
			after->next->prev = ptr;
		after->next = ptr;
	}

	struct FreeBlock
	{
		MemoryBlockData* m_block = nullptr;
		uint64_t m_freeSize = 0;

		INLINE FreeBlock()
			: m_block(nullptr)
			, m_freeSize(0)
		{}

		INLINE FreeBlock(MemoryBlockData* block)
			: m_block(block)
			, m_freeSize(block->size)
		{}

		INLINE FreeBlock(uint64_t freeSize)
			: m_block(nullptr)
			, m_freeSize(freeSize)
		{}

		INLINE bool operator==(const FreeBlock& other) const
		{
			return (m_freeSize == other.m_freeSize) && (m_block == other.m_block);
		}

		INLINE bool operator<(const FreeBlock& other) const
		{
			if (m_freeSize != other.m_freeSize)
				return m_freeSize < other.m_freeSize;

			if (other.m_block && m_block)
				return m_block->address < other.m_block->address;

			return false;
		}

		INLINE bool canAllocate(uint32_t size, uint32_t alignment) const
		{
			auto alignedOffset  = Align(m_block->address, range_cast<uint64_t>(alignment));
			auto endOffset  = m_block->address + m_block->size;
			return alignedOffset + size <= endOffset;
		}
	};

	StructureAllocator<MemoryBlockData> m_blockPool; // linked list of all blocks in the pool, very slow to visit directly
	MemoryBlockData* m_blockList = nullptr; // list of all blocks ordered by the offset

	uint64_t m_baseAddress = 0; // base address of the pool
	uint64_t m_totalSize = 0; // total size of memory we manage
	uint32_t m_minFreeBlockSize = 0; // free blocks below this size are not tracked and cannot be allocated

	Array<FreeBlock> m_freeBlocks; // free blocks sorted by their size (larges to smallest)

	int firstBestFreeBlock(uint32_t size, uint32_t alignment) const;
	MemoryBlockData* extractUsedBlock(MemoryBlockData* freeBlock, uint64_t splitStart, uint64_t splitEnd);

	void validate() const;

	///--

	uint64_t m_numAllocatedBytes = 0;
	uint64_t m_maxAllocatedBytes = 0;
	uint32_t m_numAllocatedBlocks = 0;
	uint32_t m_maxAllocatedBlocks = 0;
};

END_INFERNO_NAMESPACE()

 