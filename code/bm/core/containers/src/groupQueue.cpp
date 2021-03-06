/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "groupQueue.h"

BEGIN_INFERNO_NAMESPACE();

//---

GroupQueue::GroupQueue()
    : m_entryFreeList(sizeof(Entry) * 1024)
    , m_orderBucketFreeList(sizeof(OrderBucket) * 1024)
{
}

GroupQueue::~GroupQueue()
{
}

void GroupQueue::push(void* jobData, uint64_t order)
{
    auto* entry = m_entryFreeList.create();
    entry->payload = jobData;
    entry->next = nullptr;
    //entry->prev = nullptr;

    auto* bucket = getBucket(order);
    if (bucket->tail)
    {
        //entry->prev = bucket->tail;
        bucket->tail->next = entry;
        bucket->tail = entry;
    }
    else
    {
        bucket->tail = entry;
        bucket->head = entry;
    }
}

#if 0
void* GroupQueue::pop()
{
    // look for the entry in the ordered buckets
    for (auto* bucket = m_orderList.head; bucket; bucket = bucket->next)
    {
        if (auto* entry = bucket->head)
        {
            bucket->head = entry->next;
            if (!bucket->head)
                bucket->tail = nullptr;

            /*if (bucket->head)
                bucket->head->prev = nullptr;
            else
                bucket->tail = nullptr;*/

            auto* ret = entry->payload;
            m_entryFreeList.free(entry);

            if (bucket->head == nullptr && !bucket->hot)
            {
                unlinkBucket(bucket);
                m_orderBucketFreeList.free(bucket);
            }

            return ret;
        }
    }

    // noting found
    return nullptr;
}
#endif

bool GroupQueue::peek(std::function<PeekResult(void* top)> func)
{
	// look for the entry in the ordered buckets
    auto* bucket = m_orderList.head;
	while (bucket)
	{
        auto bucketNext = bucket->next;

        auto** pprev = &bucket->head;
        auto* entry = bucket->head;
		while (entry)
		{
            const auto result = func(entry->payload);

            // stop now without doing anything to the entry, queue remains unchanged
            if (result == PeekResult::Keep)
                return true;

            // continue to next entry and recheck
            if (result == PeekResult::Continue)
            {
                pprev = &entry->next;
                entry = entry->next;
                continue;
            }

            // remove this entry
            *pprev = entry->next;
			if (!bucket->head)
				bucket->tail = nullptr;

			/*bucket->head = entry->next;
			if (bucket->head)
				bucket->head->prev = nullptr;
			else
				bucket->tail = nullptr;*/

			m_entryFreeList.free(entry);

            // if bucket became empty unlink it
			if (bucket->head == nullptr && !bucket->hot)
			{
				unlinkBucket(bucket);
				m_orderBucketFreeList.free(bucket);
			}

            // entry was consumed as well
			return true;
		}


        bucket = bucketNext;
	}

	// noting found
	return false;
}

void GroupQueue::inspect(const std::function<void(void*)>& inspector) const
{
    // TODO!!!
}

//--

void GroupQueue::unlinkBucket(OrderBucket* bucket)
{
    if (bucket->next)
    {
        ASSERT(m_orderList.tail != bucket);
        bucket->next->prev = bucket->prev;
    }
    else
    {
        ASSERT(m_orderList.tail == bucket);
        m_orderList.tail = bucket->prev;
    }

    if (bucket->prev)
    {
        ASSERT(m_orderList.head != bucket);
        bucket->prev->next = bucket->next;
    }
    else
    {
        ASSERT(m_orderList.head == bucket);
        m_orderList.head = bucket->next;
    }

    uint64_t prevOrder = 0;
    for (auto* cur = m_orderList.head; cur; cur = cur->next)
    {
        DEBUG_CHECK(prevOrder < cur->order);
        DEBUG_CHECK(cur->next || cur == m_orderList.tail);
        DEBUG_CHECK(cur->prev || cur == m_orderList.head);
        prevOrder = cur->order;
    }
}

void GroupQueue::linkBucket(OrderBucket* bucket)
{
    DEBUG_CHECK(bucket->next == nullptr);
    DEBUG_CHECK(bucket->prev == nullptr);
    DEBUG_CHECK(bucket->head == nullptr);
    DEBUG_CHECK(bucket->tail == nullptr);

    if (m_orderList.head == nullptr)
    {
        m_orderList.head = bucket;
        m_orderList.tail = bucket;
        return;
    }

    auto* addBefore = m_orderList.head;
    while (addBefore && bucket->order > addBefore->order)
        addBefore = addBefore->next;

    if (addBefore)
    {
        if (auto* addAfter = addBefore->prev)
        {
            bucket->prev = addAfter;
            bucket->next = addBefore;
            addAfter->next = bucket;
            addBefore->prev = bucket;
        }
        else
        {
            if (m_orderList.head)
                m_orderList.head->prev = bucket;
            bucket->next = m_orderList.head;
            m_orderList.head = bucket;
        }
    }
    else
    {
        if (m_orderList.tail)
            m_orderList.tail->next = bucket;
        bucket->prev = m_orderList.tail;
        m_orderList.tail = bucket;
    }

    uint64_t prevOrder = 0;
    for (auto* cur = m_orderList.head; cur; cur = cur->next)
    {
        DEBUG_CHECK(prevOrder < cur->order);
        DEBUG_CHECK(cur->next || cur == m_orderList.tail);
        DEBUG_CHECK(cur->prev || cur == m_orderList.head);
        prevOrder = cur->order;
    }
}

GroupQueue::OrderBucket* GroupQueue::getBucket(uint64_t orderList)
{
    // check in the hot buckets
    uint32_t minBucketIndex = 0;
    uint64_t minBucketValue = ~0ULL;
    for (uint32_t i = 0; i < OrderList::MAX_HOT_BUCKETS; ++i)
    {
        const auto& hot = m_orderList.hotBuckets[i];
        if (hot.order == orderList && hot.bucket)
            return hot.bucket;

        if (hot.order < minBucketValue)
        {
            minBucketValue = hot.order;
            minBucketIndex = i;
        }
    }

    // if we are evicting bucket and it has no jobs we can remove it
    auto& evictedHot = m_orderList.hotBuckets[minBucketIndex];
    if (auto* bucketToEvict = evictedHot.bucket)
    {
        DEBUG_CHECK(bucketToEvict->hot);
        bucketToEvict->hot = false;

        evictedHot.bucket = nullptr;

        if (!bucketToEvict->head)
        {
            unlinkBucket(bucketToEvict);
            m_orderBucketFreeList.free(bucketToEvict);
        }
    }

    // do a linear search to find existing bucket
    auto* bucket = m_orderList.head;
    while (nullptr != bucket)
    {
        if (bucket->order == orderList)
            break;
        bucket = bucket->next;
    }

    // no matching bucket found, create a new one
    if (!bucket)
    {
        bucket = m_orderBucketFreeList.create();
        bucket->order = orderList;
        bucket->next = nullptr;
        bucket->prev = nullptr;
        bucket->head = nullptr;
        bucket->tail = nullptr;
        bucket->hot = false;
        linkBucket(bucket);
    }

    // make hot
    DEBUG_CHECK(!bucket->hot);
    bucket->hot = true;

    // replace in hot list
    evictedHot.bucket = bucket;
    evictedHot.order = orderList;
    return bucket;
}

//---

END_INFERNO_NAMESPACE()
