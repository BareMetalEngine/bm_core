/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types #]
***/

#pragma once

#include "rttiType.h"

BEGIN_INFERNO_NAMESPACE()

//---

/// RTTI-side base interface for a resource reference type, main purpose of having it here it facilitate resource reloading
class BM_CORE_OBJECT_API IResourceReferenceType : public IType
{
public:
    IResourceReferenceType(StringID name);
    virtual ~IResourceReferenceType();

    //--

    /// get the class we are pointing to
    virtual ClassType referenceResourceClass() const = 0;

    /*/// read the currently referenced resource
    virtual void referenceReadResource(const void* data, RefPtr<IResource>& outRef) const = 0;

    /// write new resource reference
    virtual void referenceWriteResource(void* data, IResource* resource) const = 0;*/

    /// patch resource reference, returns true if indeed it was patched
    virtual bool referencePatchResource(void* data, IResource* currentResource, IResource* newResources) const = 0;
};

//---

END_INFERNO_NAMESPACE()
