/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: instancing #]
***/

#include "build.h"
#include "instanceBufferLayout.h"
#include "instanceBuffer.h"

#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

InstanceBufferLayout::InstanceBufferLayout()
    : m_size(0)
    , m_alignment(0)
    , m_complexVarsList(nullptr)
{
}

InstanceBufferLayout::~InstanceBufferLayout()
{
}

void InstanceBufferLayout::initializeBuffer(void* bufferMemory) const
{
    memzero(bufferMemory, m_size);

    auto var  = m_complexVarsList;
    while (var != nullptr)
    {
        auto varData  = OffsetPtr<void>(bufferMemory, var->m_offset);
        var->m_type->construct(varData);
        var = var->m_nextComplexType;
    }
}

void InstanceBufferLayout::destroyBuffer(void* bufferMemory) const
{
    auto var  = m_complexVarsList;
    while (var != nullptr)
    {
        auto varData  = OffsetPtr<void>(bufferMemory, var->m_offset);
        var->m_type->destruct(varData);
        var = var->m_nextComplexType;
    }
}

void InstanceBufferLayout::copyBufer(void* destBufferMemory, const void* srcBufferMemory) const
{
    memcpy(destBufferMemory, srcBufferMemory, m_size);

    auto var  = m_complexVarsList;
    while (var != nullptr)
    {
        auto varData  = OffsetPtr<void>(destBufferMemory, var->m_offset);
        var->m_type->construct(varData);

        auto srcVarData  = OffsetPtr<void>(srcBufferMemory, var->m_offset);
        var->m_type->copy(varData, srcVarData);
        var = var->m_nextComplexType;
    }
}

InstanceBufferPtr InstanceBufferLayout::createInstance(const char* tag)
{
    if (!tag || !*tag)
        tag = "InstanceBuffer";

    void* ptr = Memory::AllocateBlock(size(), alignment(), tag);
    initializeBuffer(ptr);

    return RefNew<InstanceBuffer>(AddRef(this), ptr, size(), tag);
}

//--

END_INFERNO_NAMESPACE()

