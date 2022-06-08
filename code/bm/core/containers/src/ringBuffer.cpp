/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "ringBuffer.h"

BEGIN_INFERNO_NAMESPACE()

//--

void RingBufferBlock::print(IFormatStream& f) const
{
    if (m_size == 0)
        f << "empty";
    else
        f.appendf("offset {}, size {}", m_offset, m_size);
}

//--

RingBuffer::RingBuffer()
{}

RingBuffer::~RingBuffer()
{}

void RingBuffer::init(uint64_t size, uint32_t numBlocks /*= 1024*/)
{
    auto lock = CreateLock(m_lock);

    // reset tables
    m_regions.reset();
    m_regions.reserve(numBlocks);
    m_blocks.reset();
    m_blocks.reserve(numBlocks);

    // reset stats
    m_numBytes = size;
    m_numAllocatedBytes = 0;
    m_numAllocatedBlocks = 0;

    // setup free region for the whole space
    auto& region = m_regions.emplaceBack();
    region.allocated = false;
    region.next = INDEX_NONE;
    region.offset = 0;
    region.size = size;

    // initialize free list starting at the large empty block
    m_freeList = 0;
}

void RingBuffer::validateBlocks()
{

}

bool RingBuffer::coalesceFreeBlocks(int regionIndex, uint64_t& outFreeSpaceSize)
{
    DEBUG_CHECK_RETURN_EX_V(regionIndex != INDEX_NONE, "Invalid region to merge", false);

    auto& headRegion = m_regions[regionIndex];
    if (headRegion.allocated) // head region is allocated
        return false;

    bool freeListCoalesced = false;

    auto follower = headRegion.next;
    while (follower != INDEX_NONE)
    {
        auto& followerRegion = m_regions[follower];
        if (followerRegion.allocated)
            break;

        if (follower == m_freeList)
            freeListCoalesced = true;

        ASSERT_EX(followerRegion.offset == headRegion.offset + headRegion.size, "Region list inconsistency");
        headRegion.size += followerRegion.size;
        headRegion.next = followerRegion.next;

        m_unallocatedRegionIndices.pushBack(follower); // region can be reused
        follower = followerRegion.next;

        followerRegion.size = 0;
        followerRegion.offset = 0;
        followerRegion.next = INDEX_NONE;
    }

    if (freeListCoalesced)
        m_freeList = regionIndex;

    outFreeSpaceSize = headRegion.size;
    return true;
}

uint32_t RingBuffer::placeRegion(const RegionInfo& info)
{
    if (!m_unallocatedRegionIndices.empty())
    {
        auto index = m_unallocatedRegionIndices.back();
        m_unallocatedRegionIndices.popBack();
        m_regions[index] = info;
        return index;
    }
    else
    {
        m_regions.pushBack(info);
        return m_regions.lastValidIndex();
    }
}

int RingBuffer::cutRegion(int regionIndex, uint64_t size)
{
    auto& region = m_regions[regionIndex];
    ASSERT_EX(!region.allocated, "Region already allocated");
    ASSERT_EX(region.size <= size, "Region is to small for allocation");

    // mark as allocated
    const auto originalRegionSize = region.size;
    region.allocated = true;

    // if there's any space left create additional free region
    if (size > region.size)
    {
        region.size = size;

        RegionInfo freeRegionInfo;
        freeRegionInfo.allocated = false;
        freeRegionInfo.offset = region.offset + region.size;
        freeRegionInfo.size = originalRegionSize - region.size;
        freeRegionInfo.next = region.next;

        int nextFreeRegion = placeRegion(freeRegionInfo);
        m_regions[regionIndex].next = nextFreeRegion;
        return nextFreeRegion;
    }

    // allocated size matches region size exactly, look for next free region
    else
    {
        int nextFreeRegion = region.next;

        while (nextFreeRegion != INDEX_NONE)
        {
            const auto& nextRegion = m_regions[nextFreeRegion];
            if (nextRegion.allocated)
                break;

            nextFreeRegion = nextRegion.next;
        }

        return nextFreeRegion; // MAY be -1
    }
}

uint32_t RingBuffer::placeBlock(const BlockInfo& info)
{
    if (!m_unallocatedBlockIndices.empty())
    {
        auto index = m_unallocatedBlockIndices.back();
        m_unallocatedBlockIndices.popBack();
        m_blocks[index] = info;
        return index;
    }
    else
    {
        m_blocks.pushBack(info);
        return m_blocks.lastValidIndex();
    }
}

bool RingBuffer::makeBlock(RingBufferBlock& outBlock, int regionIndex)
{
    const auto& region = m_regions[regionIndex];
    ASSERT_EX(region.allocated, "Block must be allocated");
    ASSERT_EX(outBlock.empty(), "Block handle is not empty");

    BlockInfo info;
    info.generation = m_generationCounter++;
    info.offset = region.offset;
    info.size = region.size;
    info.region = regionIndex;

    auto blockIndex = placeBlock(info);

    outBlock.m_offset = region.offset;
    outBlock.m_size = region.size;
    outBlock.m_blockGeneration = info.generation;
    outBlock.m_blockIndex = blockIndex;

    m_numAllocatedBlocks += 1;
    m_numAllocatedBytes += region.size;

    return true;
}

bool RingBuffer::allocateBlock(uint64_t size, RingBufferBlock& outBlock, uint64_t* outMaximumSizeThatCanBeAllocated)
{
    auto lock = CreateLock(m_lock);

    // make sure the region 0 is not moved
    ASSERT_EX(m_regions[0].offset == 0, "Root region must be always at offset 0");

    // coalesce memory at head
    uint64_t maxFreeSizeAtHead = 0;
    coalesceFreeBlocks(0, maxFreeSizeAtHead);

    // check if we have free space at the current location
    uint64_t maxFreeSizeAtCur = 0;
    if (m_freeList > 0)
        coalesceFreeBlocks(m_freeList, maxFreeSizeAtCur);

    // allocate from free list if possible
    if (maxFreeSizeAtCur >= size)
    {
        auto regionIndex = m_freeList;
        m_freeList = cutRegion(m_freeList, size);
        return makeBlock(outBlock, regionIndex);
    }

    // free list is to small but maybe we can allocate from head of space
    if (maxFreeSizeAtHead >= size)
    {
        auto regionIndex = 0;
        m_freeList = cutRegion(0, size); // wrap around the free list
        return makeBlock(outBlock, regionIndex);
    }

    // allocation failed
    if (outMaximumSizeThatCanBeAllocated) // report size that could be allocated
        *outMaximumSizeThatCanBeAllocated = std::max<uint64_t>(maxFreeSizeAtHead, maxFreeSizeAtCur);
    return false;
}

void RingBuffer::freeBlock(const RingBufferBlock& handle, uint64_t* outMaximumSizeThatCanBeAllocated /*= nullptr*/)
{
    DEBUG_CHECK_RETURN_EX(handle, "Trying to free invalid block");

    auto lock = CreateLock(m_lock);

    DEBUG_CHECK_RETURN_EX(handle.m_blockIndex < m_blocks.size(), "Invalid block index");
    auto& block = m_blocks[handle.m_blockIndex];
    DEBUG_CHECK_RETURN_EX(block.generation != handle.m_blockGeneration, "Ringbuffer block handle has invalid generation number");
    DEBUG_CHECK_RETURN_EX(block.offset != handle.m_offset, "Ringbuffer block handle has invalid offset");
    DEBUG_CHECK_RETURN_EX(block.size != handle.m_size, "Ringbuffer block handle has invalid size");

    DEBUG_CHECK_RETURN_EX(block.region >= 0, "Block is not allocated");
    DEBUG_CHECK_RETURN_EX(block.region <= m_regions.lastValidIndex(), "Block has invalid region index");

    auto& region = m_regions[block.region];
    DEBUG_CHECK_RETURN_EX(region.allocated, "Region is not allocated");
    DEBUG_CHECK_RETURN_EX(region.size == handle.m_size, "Region has invalid size");
    DEBUG_CHECK_RETURN_EX(region.offset == handle.m_offset, "Region has invalid offset");

    region.allocated = false; // mark as free

    m_numAllocatedBlocks -= 1;
    m_numAllocatedBytes -= region.size;

    block.region = INDEX_NONE;
    block.offset = 0;
    block.size = 0;
    block.generation = 0;

    m_unallocatedBlockIndices.pushBack(handle.m_blockIndex); // reuse block

    if (outMaximumSizeThatCanBeAllocated)
    {
        // coalesce memory at head
        uint64_t maxFreeSizeAtHead = 0;
        coalesceFreeBlocks(0, maxFreeSizeAtHead);

        // check if we have free space at the current location
        uint64_t maxFreeSizeAtCur = 0;
        if (m_freeList > 0)
            coalesceFreeBlocks(m_freeList, maxFreeSizeAtCur);

        *outMaximumSizeThatCanBeAllocated = std::max<uint64_t>(maxFreeSizeAtHead, maxFreeSizeAtCur);
    }
}

//--

END_INFERNO_NAMESPACE()
