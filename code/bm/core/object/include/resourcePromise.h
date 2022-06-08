/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "resourceId.h"

BEGIN_INFERNO_NAMESPACE()

//-----

// Promise of resource that is being loaded and is going to be loaded at some point
class BM_CORE_OBJECT_API ResourcePromise : public IReferencable
{
public:
    ResourcePromise(ResourceID id, ResourceClass cls, StringBuf path="", IResource* ptr = nullptr);
    virtual ~ResourcePromise();

    //---

	/// requested class of the resource
	INLINE const ResourceClass& cls() const { return m_cls; }

    /// ID of the promised resource
    INLINE const ResourceID& id() const { return m_id; } 

    /// loading path (may not be set)
    INLINE const StringBuf& path() const { return m_path; }

    /// is the promise fulfilled
    INLINE bool fullfilled() const { return m_fullfilled; }

    //---

    /// resolve the loaded resource
    ResourcePtr resolve() const;

    /// inject value
    void fulfill(IResource* resource);

    //---

    /// create empty promise
    static ResourcePromisePtr CreateEmptyPromise(ResourceID id, ResourceClass cls, StringBuf path="");
    static ResourcePromisePtr CreateEmptyPromise(const SerializationResourceKey& key);

    //---

private:
    ResourceID m_id;
    ResourceClass m_cls;

    StringBuf m_path;
    std::atomic<IResource*> m_ptr;
    std::atomic<bool> m_fullfilled;
};

//-----

END_INFERNO_NAMESPACE()
