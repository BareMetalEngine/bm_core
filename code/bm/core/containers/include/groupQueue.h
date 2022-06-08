/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/structureAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

/// A queue with "generation" grouping
class BM_CORE_CONTAINERS_API GroupQueue : public NoCopy
{
public:
    GroupQueue();
    ~GroupQueue();

    //--

    ///! push entry to the queue with given generation index, when popping the entries from lower generation are preferred
    void push(void* entry, uint64_t generation);

    ///! pop a job from the queue, the entries from lower generation are preferred
    ///! returns NULL if there's nothing to pop (queue is empty)
    //void* pop();

    //--

    enum class PeekResult : uint8_t
    {
        Keep, // keep existing entry (consumes element)
        Remove, // remove existing entry (consumes element)
		Continue, // ignore existing entry, continue search (does not consume element)
    };

    ///! peak at the head element, returns true if we consume an element
    bool peek(std::function<PeekResult(void* top)> func);

    //--

    ///! iterate over all elements in the queue
    void inspect(const std::function<void(void*)>& inspector) const;

    //--

private:
    struct Entry
    {
        void* payload = nullptr;
        Entry* next = nullptr;
        //Entry* prev = nullptr;
    };

    struct OrderBucket
    {
        uint64_t order = 0;
        bool hot = false;
        OrderBucket* next = nullptr;
        OrderBucket* prev = nullptr;

        Entry* head = nullptr;
        Entry* tail = nullptr;

        void push(Entry* entry);
        void unlink(Entry* entry);
        Entry* pop(uint32_t mask);
        void inspect(const std::function<void(void*)>& inspector) const;
    };

    //--

    struct HotOrderListEntry
    {
        uint64_t order = 0;
        OrderBucket* bucket = nullptr;
    };

    struct OrderList
    {
        OrderBucket* head = nullptr;
        OrderBucket* tail = nullptr;

        static const auto MAX_HOT_BUCKETS = 8;
        HotOrderListEntry hotBuckets[MAX_HOT_BUCKETS];
    };

    void unlinkBucket(OrderBucket* bucket);
    void linkBucket(OrderBucket* bucket);
    OrderBucket* getBucket(uint64_t orderList);

    //--

    OrderList m_orderList;

    StructureAllocator<Entry> m_entryFreeList;
    StructureAllocator<OrderBucket> m_orderBucketFreeList;
};

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()
