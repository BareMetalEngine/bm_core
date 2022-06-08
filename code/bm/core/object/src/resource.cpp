/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "resource.h"

BEGIN_INFERNO_NAMESPACE()

//---

static std::atomic<ResourceUniqueID> GResourceUniqueID(1);

//---

RTTI_BEGIN_TYPE_ABSTRACT_CLASS(IResource);
    //RTTI_METADATA(ResourceDescriptionMetadata).description("Resource");
    RTTI_SCRIPT_CLASS_FUNCTION_EX("get_LoadPath", _GetLoadPath);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("get_RuntimeUniqueId", _GetRuntimeUniqueId);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("get_ModifiedFlag", _GetModifiedFlag);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("MarkModified", markModified);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ResetModifiedFlag", resetModifiedFlag);
RTTI_END_TYPE();

IResource::IResource()
{
    m_runtimeUniqueId = ++GResourceUniqueID;
}

IResource::~IResource()
{
}

void IResource::resetModifiedFlag()
{
    if (m_modified)
    {
        m_modified = false;

        auto selfRef = ResourcePtr(AddRef(this));
        DispatchGlobalEvent(eventKey(), EVENT_RESOURCE_MODIFIED, selfRef);
    }
}

void IResource::markModified()
{
    // this resource may be contained inside other file, we still need to propagate
    TBaseClass::markModified();

    // mark as modified only if we are standalone resource
    m_modified = true;

    // notify listeners
    auto selfRef = ResourcePtr(AddRef(this));
    DispatchGlobalEvent(eventKey(), EVENT_RESOURCE_MODIFIED, selfRef);
}

//--

StringBuf IResource::GetResourceDescriptionForClass(ClassType resourceClass)
{
    if (!resourceClass)
        return StringBuf("None");

    /*auto descMetaData  = resourceClass->findMetadata<ResourceDescriptionMetadata>();
    if (descMetaData && descMetaData->description() && *descMetaData->description())
        return StringBuf(descMetaData->description());

    if (resourceClass->shortName())
        return StringBuf(resourceClass->shortName().view());

    return StringBuf(resourceClass->name().view().afterLast("::"));*/

    return StringBuf("Resource");
}

void IResource::discardEditorData()
{
}

void IResource::preloadEditorData()
{
}

bool IResource::calcResourceStreamingDistance(float& outDistance) const
{
    return false;
}

void IResource::bindLoadPath(StringView path)
{
    m_loadPath = StringBuf(path);
}

//--

StringBuf IResource::_GetLoadPath() const
{
    return loadPath();
}

uint64_t IResource::_GetRuntimeUniqueId() const
{
    return runtimeUniqueId();
}

bool IResource::_GetModifiedFlag() const
{
    return modified();
}

//--

END_INFERNO_NAMESPACE()
