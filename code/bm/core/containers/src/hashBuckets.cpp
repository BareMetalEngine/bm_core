/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "hashBuckets.h"

BEGIN_INFERNO_NAMESPACE()

//--

HashBuckets::HashBuckets()
{}

void HashBuckets::Reset(HashBuckets* data)
{
    if (data)
        memset(data->m_buckets, 0xFF, data->m_bucketCount * sizeof(int));
}

void HashBuckets::Clear(HashBuckets*& data)
{
    Memory::FreeBlock(data);
    data = nullptr;
}

bool HashBuckets::CheckCapacity(const HashBuckets* data, uint32_t elementCount)
{
    if (elementCount < MIN_BUCKETS)
        return true;

    if (data && elementCount < data->m_capacity)
        return true;

    return false;
}

//--

END_INFERNO_NAMESPACE()

