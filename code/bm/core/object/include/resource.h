/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "object.h"

BEGIN_INFERNO_NAMESPACE()

//-----

DECLARE_GLOBAL_EVENT(EVENT_RESOURCE_MODIFIED, ResourcePtr);

//-----

// Base resource class
// Resource is a fancy object written to file that can point to other resources in other files
class BM_CORE_OBJECT_API IResource : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(IResource, IObject);

public:
    //--

    static inline StringView FILE_EXTENSION = "xfile";

    //--

    // File path used to load the resource - NOTE, debug only
    INLINE const StringBuf& loadPath() const { return m_loadPath; }

    // Get runtime only unique ID, allows to identify resources by simpler means than just the number
    // NOTE: the ID gets reset if the resource is reloaded
    INLINE ResourceUniqueID runtimeUniqueId() const { return m_runtimeUniqueId; }

    // Is the resource considered modified ?
    INLINE bool modified() const { return m_modified; }

    //--

    IResource();
    virtual ~IResource();

    // mark resource as modified
    // NOTE: this propagates the event to the loader
    // NOTE: the isModified flag is NOT stored in the resource but on the side of the managing structure (like ManagedDepot)
    virtual void markModified() override;

    // reset resource modified flag
    void resetModifiedFlag();

    // bind source load path
    void bindLoadPath(StringView path);

    //---

    // Get resource description for given class
    static StringBuf GetResourceDescriptionForClass(ClassType resourceClass);

    //---

    // Drop any editor only data for this resource
    // Called when building deployable packages
    virtual void discardEditorData();

    // preload all dependent content, used in case of editor streaming
    virtual void preloadEditorData();

    //---

    // Retrieve the loading (streaming) distance for this resource
    // NOTE: this is implementation specific
    // NOTE: returns false if there's no well determined streaming distance
    virtual bool calcResourceStreamingDistance(float& outDistance) const;

    //--

private:
    StringBuf m_loadPath; // path we loaded this resource from, debug only

    ResourceUniqueID m_runtimeUniqueId = 0; // resource runtime unique ID, can be used to index maps instead of pointer

    ResourcePtr m_reloadedData; // new version of this resource

    bool m_modified = false;

    //--

    StringBuf _GetLoadPath() const;
    uint64_t _GetRuntimeUniqueId() const;
    bool _GetModifiedFlag() const;
};

//---

END_INFERNO_NAMESPACE()
