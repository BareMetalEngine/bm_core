/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

class RingBuffer;

/// Region in the ring buffer
class BM_CORE_CONTAINERS_API RingBufferBlock
{
    friend class RingBuffer;

public:
    INLINE RingBufferBlock() = default;
    INLINE RingBufferBlock(std::nullptr_t) {};
    INLINE RingBufferBlock(const RingBufferBlock& other) = default;
    INLINE RingBufferBlock& operator=(const RingBufferBlock& other) = default;

    INLINE operator bool() const { return m_size != 0; }

    INLINE bool empty() const { return m_size == 0; }

    INLINE uint64_t offset() const { return m_offset; }
    INLINE uint64_t size() const { return m_size; }

    void print(IFormatStream& f) const;

private:
    uint64_t m_offset = 0; // offset to allocated region
    uint64_t m_size = 0; // size of allocated region

    uint32_t m_blockIndex = 0; // internal index in the ring buffer table
    uint32_t m_blockGeneration = 0; // sentinel to detect any corruption
};

//---

/// Ring buffer of memory that can be sub-allocated in a circular fashion
/// The allocated blocks can be released in any order 
/// NOTE: this structure IS internally synchronized 
class BM_CORE_CONTAINERS_API RingBuffer : public MainPoolData<NoCopy>
{
public:
    RingBuffer();
    ~RingBuffer();

    //--

    // total buffer size
    INLINE uint64_t bufferSize() const { return m_numBytes; }

    // size of all allocated blocks
    INLINE uint64_t allocatedSize() const { return m_numAllocatedBytes.load(); }

    // number of allocated blocks
    INLINE uint32_t allocatedBlocks() const { return m_numAllocatedBlocks.load(); }

    //--

    // initialize ring buffer
    // NOTE: ring buffer does NOT own any memory, it only manages address space use
    void init(uint64_t size, uint32_t numBlocks = 1024);

    //--

    // allocate space from ring buffer, returns empty block if space cannot be allocated and maximum size that can be allocated
    bool allocateBlock(uint64_t size, RingBufferBlock& outBlock, uint64_t* outMaximumSizeThatCanBeAllocated = nullptr);

    // free previously allocated space from the ring buffer, returns size of maximum size that can be allocated now after free
    void freeBlock(const RingBufferBlock& block, uint64_t* outMaximumSizeThatCanBeAllocated = nullptr);

    //--

private:
    struct BlockInfo
    {
        uint32_t generation = 0;
        uint64_t offset = 0;
        uint64_t size = 0;
        int region = INDEX_NONE;
    };

    struct RegionInfo
    {
        bool allocated = false;
        int next = INDEX_NONE;
        uint64_t offset = 0;
        uint64_t size = 0;
    };

    uint64_t m_numBytes = 0;
    std::atomic<uint64_t> m_numAllocatedBytes;
    std::atomic<uint32_t> m_numAllocatedBlocks;

    SpinLock m_lock;
    Array<BlockInfo> m_blocks;
    Array<uint32_t> m_unallocatedBlockIndices;

    Array<RegionInfo> m_regions;
    Array<uint32_t> m_unallocatedRegionIndices;

    int m_freeList = -1; // can become empty...

    uint32_t m_generationCounter = 1;

    //--

    void validateBlocks();
    bool coalesceFreeBlocks(int regionIndex, uint64_t& outFreeSpaceSize);
    int cutRegion(int regionIndex, uint64_t size);
    bool makeBlock(RingBufferBlock& outBlock, int region);

    uint32_t placeRegion(const RegionInfo& info);
    uint32_t placeBlock(const BlockInfo& info);
};

//---

END_INFERNO_NAMESPACE()
