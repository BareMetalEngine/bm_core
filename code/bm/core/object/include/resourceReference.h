/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "resourceId.h"

BEGIN_INFERNO_NAMESPACE()

///--------

/// base concept of serializable resource reference
class BM_CORE_OBJECT_API BaseReference
{
public:
    INLINE BaseReference() = default;
    INLINE BaseReference(std::nullptr_t) {};
	INLINE ~BaseReference() {}

    explicit BaseReference(const IResource* ptr); // inplace
    explicit BaseReference(const ResourcePromise* promise); // external
    
    BaseReference(const BaseReference& other);
    BaseReference(BaseReference&& other);
    
    BaseReference& operator=(const BaseReference& other);
    BaseReference& operator=(BaseReference&& other);

    ///--

    // Is this inlined resource? Inlined resource is "embedded" without actually being loaded
    INLINE bool inlined() const { return m_ptr != nullptr; }

	// Is this external resource?
	INLINE bool external() const { return m_promise != nullptr; }

    // is this an empty reference ? empty reference is one without key and without object
    INLINE bool empty() const { return m_ptr == nullptr && m_promise == nullptr; }

    // validate if reference is set (NOTE: we can still have no data)
    INLINE operator bool() const { return !empty(); }

    //---

    // Resolve the ID of the references resource
    ResourceID resolveId() const;

    // Resolve class of resource
    ResourceClass resolveClass() const;

	//! Get pointer to the resource
	ResourcePtr resolveResource() const;

    // Try to get path for the resource
    StringBuf resolvePath() const;

    //---

    // clear reference
    void reset();

    //--

    /// test references for equality, only PATHS are checked
    bool operator==(const BaseReference& other) const;
    INLINE bool operator!=(const BaseReference& other) const { return !operator==(other); }

    //--

    // print to text
    // for a key reference it prints: "Texture$engine/textures/lena.png"
    // for inlined objects this just prints: "Texture"
    void print(IFormatStream& f) const;

    //--

    // hash map integration - uses resource ID if possible
    static uint32_t CalcHash(const BaseReference& ref);

protected:
    ResourcePtr m_ptr;
    ResourcePromisePtr m_promise;
};

///--------

/// Reference to resource, preserves the path even if the actual resource was not loaded (errors)
/// NOTE: use ResourceLoadingService to load resources and resource handles
template< class T >
class ResourceRef : public BaseReference
{
public:
    INLINE ResourceRef() = default;
    INLINE ResourceRef(std::nullptr_t) {};
    INLINE ResourceRef(ResourceRef<T>&& other) = default;
    INLINE ResourceRef& operator=(const ResourceRef<T>& other) = default;
    INLINE ResourceRef& operator=(ResourceRef<T> && other) = default;
    INLINE ResourceRef(const ResourceRef<T>& other) = default;
    INLINE ResourceRef(const BaseReference& baseRef) : BaseReference(baseRef) {}

    template< typename U >
    INLINE ResourceRef(const ResourceRef<U>& other)
        : BaseReference(other)
    {
        ASSERT_EX(0 == (int64_t) static_cast<T*>((U*)nullptr), "Address to object in the shared pointer cannot change due to case");
        static_assert(std::is_base_of<T, U>::value, "Cannot down-cast a pointer through construction, use rtti_cast");
    }

    // get resource
    INLINE RefPtr<T> resource() const { return rtti_cast<T>(BaseReference::resource()); }

    ///---

    // check for equality
    INLINE bool operator==(const ResourceRef<T>& other) const { return BaseReference::operator==(other); }

    // check for in equality
    INLINE bool operator!=(const ResourceRef<T>& other) const { return BaseReference::operator!=(other); }

    //--
 };

///--------

// type naming, generate a RTTI type name for a ResourceRef<T> with given class
extern BM_CORE_OBJECT_API StringID FormatRefTypeName(StringID className);

// get a reference type name
template< typename T >
INLINE StringID FormatRefTypeName() {
    static const auto typeName = FormatRefTypeName(T::GetStaticClass()->name());
    return typeName;
}

//--------

namespace resolve
{

    // type name resolve for strong handles
    template<typename T>
    struct TypeName<ResourceRef<T>>
    {
        static StringID GetTypeName()
        {
            static auto cachedTypeName = FormatRefTypeName(TypeName<T>::GetTypeName());
            return cachedTypeName;
        }
    };

} // resolve

//--------

END_INFERNO_NAMESPACE()

//------

