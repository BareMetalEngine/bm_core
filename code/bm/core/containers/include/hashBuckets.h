/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// general hashing helper (bucket linked list manager)
class BM_CORE_CONTAINERS_API HashBuckets : public MainPoolData<NoCopy>
{
public:
    //--

    // reset without clearing
    static void Reset(HashBuckets* data);

    // delete
    static void Clear(HashBuckets*& data);

    // check if helper has enough capacity
    static bool CheckCapacity(const HashBuckets* data, uint32_t elementCount);

    //--

    // find element index
    template< typename K, typename FK >
    static bool Find(const HashBuckets* helper, const K* keys, const uint32_t keyCount, const FK& searchKey, uint32_t& outKeyIndex);

    // insert element link
    template< typename K >
    static void Insert(HashBuckets* helper, const K& key, uint32_t index);

    // remove element entry, does not do any rehashing
    template< typename K, typename FK  >
    static bool Remove(HashBuckets* helper, const K* keys, const uint32_t keyCount, const FK& searchKey, uint32_t& outKeyIndex);

    // remove element entry in an ordered way (all indices > element index will have to be rehashes)
    // this is much slower then Remove but preserves order of entries
    template< typename K, typename FK  >
    static bool RemoveOrdered(HashBuckets* helper, const K* keys, const uint32_t keyCount, const FK& searchKey, uint32_t& outKeyIndex);

    //--

    template< typename K >
    static void Build(HashBuckets*& helper, const K* keys, uint32_t keysCount, uint32_t keysCapacity);

    //--

private:
    static const inline uint32_t MIN_BUCKETS = 64; // no buckets are allocated if we have less than this number of elements
    static const inline uint32_t MAX_BUCKETS = 65536;

    HashBuckets();
    ~HashBuckets();

    uint32_t m_capacity = 0;

    uint32_t m_bucketCount = 0;
    uint32_t m_bucketMask = 0; // always Pow2 mask

    int* m_links = nullptr;

    int m_buckets[1]; // must be last

    //--

    static uint32_t BucketNextPow2(uint32_t v);
};

//--

END_INFERNO_NAMESPACE()

#include "hashBuckets.inl"