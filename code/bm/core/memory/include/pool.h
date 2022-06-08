/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

/// Stats for memory pool
struct BM_CORE_MEMORY_API PoolStats
{
	uint64_t totalSize = 0;

    uint64_t allocatedBytes = 0;
    uint64_t allocatedBlocks = 0;
	uint64_t maxAllocatedBytes = 0;
	uint64_t maxAllocatedBlocks = 0;

    uint64_t wastedBytes = 0;

    uint64_t runningAllocationCount = 0;
    uint64_t runningAllocationSize = 0;

    //--

    PoolStats();

    void print(IFormatStream& f) const;
};

///--

/// Type of memory pool
enum class PoolType : uint8_t
{
    // Blocks allocated from pool are not managed, they are just raw pointers
    // This pool can't be defragmented
    Unmanaged, 

    // Blocks allocated from pool are "handles" to actual memory
    // This pool can be defragmented
    Managed,

    // Paged pool - allocated big pages
    Paged,
};

///--

/// Abstract memory pool from which we are allocating stuff
class BM_CORE_MEMORY_API IPool : public NoCopy
{
public:
    IPool(const char* name, PoolType type);
	virtual ~IPool();

    //---

    /// get user-given name 
    INLINE const char* name() const { return m_name; }

	/// is this pool managed ? (ie - can defrag)
	INLINE PoolType type() const { return m_type; }

    /// pull current pool stats
    virtual void stats(PoolStats& outStats) const;

    /// print detailed stats
    virtual void print(IFormatStream& f, int details = 0) const;

    //--

    //! Visit all registered pools, used mostly for stats printing
    static void IteratePools(const std::function<void(IPool&)>& func);
    
    //! Print pool status to output stream
    static void PrintPools(IFormatStream& f, int details = 0);

    //--

protected:
	std::atomic<uint64_t> m_statTotalSize;
	std::atomic<uint64_t> m_statAllocatedBytes;
	std::atomic<uint64_t> m_statAllocatedBlocks;
	std::atomic<uint64_t> m_statMaxAllocatedBytes;
	std::atomic<uint64_t> m_statMaxAllocatedBlocks;
	std::atomic<uint64_t> m_statAllocationCounter;
	std::atomic<uint64_t> m_statAllocationByteCounter;

    void notifyTotalChange(int64_t change);
    void notifyAllocation(uint64_t size);
    void notifyFree(uint64_t size);

private:
    const char* m_name = nullptr;
    PoolType m_type = PoolType::Unmanaged;
    uint32_t m_index = 0;
};

///--

END_INFERNO_NAMESPACE()
