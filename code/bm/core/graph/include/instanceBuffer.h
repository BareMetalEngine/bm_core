/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: instancing #]
***/

#pragma once

#include "instanceVar.h"

BEGIN_INFERNO_NAMESPACE()

/// data holder for instanced values 
class BM_CORE_GRAPH_API InstanceBuffer : public IReferencable
{
public:
    InstanceBuffer(const InstanceBufferLayout* layout, void* data, uint32_t size, const char* tag = nullptr);
    ~InstanceBuffer();

    /// get size of the data in the instance buffer
    INLINE uint32_t size() const { return m_size; }

    /// get pointer to instance's data 
    INLINE const void* data() const { return m_data; }

    /// get pointer to instance's data 
    INLINE void* data() { return m_data; }

    /// get the buffer's layout
    INLINE const InstanceBufferLayout* layout() const { return m_layout; }

    /// resolve pointer to data
    INLINE const void* resolvePointer(const InstanceVarBase& v) const { return GetInstanceVarData(v); }

    /// resolve pointer to data
    INLINE void* resolvePointer(const InstanceVarBase& v) { return GetInstanceVarData(v); }

    //--

    /// resolve data of instance variable
    template< typename T >
    INLINE T& operator[](const InstanceVar<T>& v)
    {
        return *(T*)GetInstanceVarData(v);
    }

    /// resolve data of instance variable
    template< typename T >
    INLINE const T& operator[](const InstanceVar<T>& v) const
    {
        return *(const T*)GetInstanceVarData(v);
    }

    //--

    /// create copy of the buffer
    InstanceBufferPtr copy() const;

private:        
    void* m_data; // buffer for all of the data in the graph instance
    uint32_t m_size; // size of all the data needed for the graph instance

    InstanceBufferLayoutPtr m_layout; // layout of the data buffer
    const char* m_tag = nullptr;

    void* GetInstanceVarData(const InstanceVarBase& v);
    const void* GetInstanceVarData(const InstanceVarBase& v) const;
};

END_INFERNO_NAMESPACE()
