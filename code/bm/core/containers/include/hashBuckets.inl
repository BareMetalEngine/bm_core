/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "hash.inl"

BEGIN_INFERNO_NAMESPACE()

//--

template< typename K, typename FK >
bool HashBuckets::Find(const HashBuckets* helper, const K* keys, const uint32_t keyCount, const FK& searchKey, uint32_t& outKeyIndex)
{
    if (helper)
    {
        const auto bucketIndex = Hasher<K>::CalcHash(searchKey) & helper->m_bucketMask;

        auto entryIndex = helper->m_buckets[bucketIndex];
        while (INDEX_NONE != entryIndex)
        {
            if (keys[entryIndex] == searchKey)
            {
                outKeyIndex = entryIndex;
                return true;
            }

            entryIndex = helper->m_links[entryIndex];
        }
    }
    else
    {
        // linear search
        for (uint32_t i = 0; i < keyCount; ++i)
        {
            if (keys[i] == searchKey)
            {
                outKeyIndex = i;
                return true;
            }
        }
    }

    return false;
}

// insert element link
template< typename K >
void HashBuckets::Insert(HashBuckets* helper, const K& key, uint32_t index)
{
    if (helper)
    {
        const auto bucketIndex = Hasher<K>::CalcHash(key) & helper->m_bucketMask;

        helper->m_links[index] = helper->m_buckets[bucketIndex];
        helper->m_buckets[bucketIndex] = index;
    }
}

// remove element entry, does not do any rehashing
template< typename K, typename FK  >
bool HashBuckets::Remove(HashBuckets* helper, const K* keys, const uint32_t keyCount, const FK& searchKey, uint32_t& outKeyIndex)
{
    if (helper)
    {
        const auto bucketIndex = Hasher<K>::CalcHash(searchKey) & helper->m_bucketMask;

        int* prevPtr = &helper->m_buckets[bucketIndex];
        while (*prevPtr != INDEX_NONE)
        {
            auto entryIndex = *prevPtr;
            if (keys[entryIndex] == searchKey)
            {
                // unlink from array in this bucket
                *prevPtr = helper->m_links[entryIndex];

                // get the index of the last element in the key/value tables as this element will have to be reindexed
                auto lastEntryIndex = keyCount - 1;
                outKeyIndex = entryIndex;

                // since we moved an element from index lastEntryIndex to entryIndex change all the stuff in the tables
                if (lastEntryIndex != entryIndex)
                {
                    // key[lastEntryIndex] will be moved to entry key[entryIndex] so fix the link as well
                    helper->m_links[entryIndex] = helper->m_links[lastEntryIndex];

                    // fixup the entry
                    const auto lastBucketIndex = Hasher<K>::CalcHash(keys[lastEntryIndex]) & helper->m_bucketMask;
                    auto* updateIndex = &helper->m_buckets[lastBucketIndex];
                    while (*updateIndex != INDEX_NONE)
                    {
                        // whatever pointed to the previous entry should new point to the new one
                        if (*updateIndex == lastEntryIndex)
                        {
                            *updateIndex = entryIndex;
                            break;
                        }

                        updateIndex = &helper->m_links[*updateIndex];
                    }
                }

                // item was removed
                return true;
            }

            // go to next item
            prevPtr = &helper->m_links[entryIndex];
        }
    }
    else
    {
        for (uint32_t i = 0; i < keyCount; ++i)
        {
            if (keys[i] == searchKey)
            {
                outKeyIndex = i;
                return true;
            }
        }
    }

    return false;
}

// remove element entry, does not do any rehashing
template< typename K, typename FK  >
bool HashBuckets::RemoveOrdered(HashBuckets* helper, const K* keys, const uint32_t keyCount, const FK& searchKey, uint32_t& outKeyIndex)
{
    if (helper)
    {
        const auto bucketIndex = Hasher<K>::CalcHash(searchKey) & helper->m_bucketMask;

        int* prevPtr = &helper->m_buckets[bucketIndex];
        while (*prevPtr != INDEX_NONE)
        {
            auto entryIndex = *prevPtr;
            if (keys[entryIndex] == searchKey)
            {
                // unlink from array in this bucket
                *prevPtr = helper->m_links[entryIndex];
                outKeyIndex = entryIndex;

                // all links with index >= entryIndex has to be decremented
                auto* readPtr = helper->m_links;
                auto* writePtr = helper->m_links;
                for (uint32_t i = 0; i < keyCount; ++i)
                {
                    auto index = *readPtr++;
                    ASSERT(index != entryIndex);
                    if (index > entryIndex)
                        index -= 1;

                    if (i != entryIndex)
                        *writePtr++ = index;
                }

                // all indices in buckets have to be updated as well
                for (uint32_t i = 0; i < helper->m_bucketCount; ++i)
                {
                    ASSERT(helper->m_buckets[i] != entryIndex);
                    if (helper->m_buckets[i] > entryIndex)
                        helper->m_buckets[i] -= 1;
                }

                // item was removed
                return true;
            }

            // go to next item
            prevPtr = &helper->m_links[entryIndex];
        }
    }
    else
    {
        for (uint32_t i = 0; i < keyCount; ++i)
        {
            if (keys[i] == searchKey)
            {
                outKeyIndex = i;
                return true;
            }
        }
    }

    // item was not found
    return false;
}

//--

ALWAYS_INLINE uint32_t HashBuckets::BucketNextPow2(uint32_t v)
{
    auto x = v;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

template< typename K >
struct HashBucketsType
{};

template< typename K >
void HashBuckets::Build(HashBuckets*& helper, const K* keys, uint32_t keysCount, uint32_t keysCapacity)
{
    // if we don't have many entries do not create the hashing region
    if (keysCapacity < MIN_BUCKETS && !helper)
        return;

    // calculate best number of crap we support
    auto requiredKeysCount = std::max<uint32_t>(keysCapacity, keysCapacity * 10 / 7); // add some extra space before rehashing
    auto requiredBucketCount = std::clamp<uint32_t>(BucketNextPow2(requiredKeysCount), MIN_BUCKETS, MAX_BUCKETS);

    // check if need to reallocate
    if (!helper || keysCount > helper->m_capacity || requiredBucketCount > helper->m_bucketCount)
    {
        const auto neededMemorySize = sizeof(HashBuckets) + (sizeof(int) * (requiredBucketCount + keysCapacity - 1));
#ifdef PLATFORM_HASTYPEID
        helper = (HashBuckets*) Memory::ResizeBlock(helper, neededMemorySize, 4, typeid(HashBucketsType<K>).name());
#else
        helper = (HashBuckets*)Memory::ResizeBlock(helper, neededMemorySize, 4, "HashBuckets");
#endif

        helper->m_bucketCount = requiredBucketCount;
        helper->m_bucketMask = requiredBucketCount - 1;
        helper->m_capacity = keysCapacity;
        helper->m_links = helper->m_buckets + helper->m_bucketCount;
    }

    // rebuild the hashing
    if (helper)
    {
        // reset the buckets
        Reset(helper);

        // insert existing keys into buckets
        for (uint32_t i = 0; i < keysCount; ++i)
        {
            const auto bucketIndex = Hasher<K>::CalcHash(keys[i]) & helper->m_bucketMask;

            helper->m_links[i] = helper->m_buckets[bucketIndex];
            helper->m_buckets[bucketIndex] = i;
        }
    }
}

//--

END_INFERNO_NAMESPACE()


