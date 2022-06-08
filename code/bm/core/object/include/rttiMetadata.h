/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\metadata #]
***/

#pragma once

#include "bm/core/containers/include/array.h"
#include "rttiClassRef.h"

BEGIN_INFERNO_NAMESPACE()

class IClassType;
class TypeSystem;

///--

/// Type metadata base class, subclasses are used to pass extra arguments to the class construction
class BM_CORE_OBJECT_API IMetadata : public MainPoolData<NoCopy>
{
public:
    virtual ~IMetadata();

    static SpecificClassType<IMetadata> GetStaticClass();
    virtual ClassType cls() const;

    //--

    // initialize class (since we have no automatic reflection in this project)
    static void RegisterType(TypeSystem& typeSystem);
};

///--

/// Metadata container
class BM_CORE_OBJECT_API MetadataContainer : public MainPoolData<NoCopy>
{
public:
    MetadataContainer();
    virtual ~MetadataContainer();

    /// remove all metadata
    void removeAllMetadata();

    /// attach meta data to type
    /// NOTE: only one given type of metadata can be defined in class, if given type is already define an existing object is reteurned
    IMetadata& addMetadata(ClassType metadataType);

    /// add already created meta data object
    void attachMetadata(IMetadata* metadata);

    /// get metadata by type, may be overridden (in classes it walks the hierarchy)
    virtual const IMetadata* metadata(ClassType metadataType) const;

    //---

    template< typename T >
    INLINE const T* findMetadata() const
    {
        static_assert(std::is_base_of<IMetadata, T>::value, "Expected IMetadata derived type");
        return static_cast<const T*>(metadata(T::GetStaticClass()));
    }

    template< typename T >
    INLINE const T& findMetadataRef() const
    {
        static_assert(std::is_base_of<IMetadata, T>::value, "Expected IMetadata derived type");
        auto metadata  = this->metadata(T::GetStaticClass());
        DEBUG_CHECK_EX(metadata != nullptr, "Crucial type's metadata is missing");
        return static_cast<const T&>(*metadata);
    }

    //----

    /// list of all meta data objects
    INLINE const Array<IMetadata*>& localMetadataList() const { return m_metadata; }

    /// collect metadata from this container and parent ones
    virtual void collectMetadataList(Array<const IMetadata*>& outMetadataList) const;

    //--

private:
    Array<IMetadata*> m_metadata;
};

//--

// short type name metadata, allows to specify a short name (without namespace bs) that can be used to find the type
// best usage is for scripting/saving type names to file where full name is just to much
// scene::mesh::Mesh -> "Mesh"
class BM_CORE_OBJECT_API ShortTypeNameMetadata : public IMetadata
{
public:
    virtual ~ShortTypeNameMetadata();

    INLINE const char* shortName() const { return m_shortName; }
    INLINE void shortName(const char* name) { m_shortName = name; }

    static SpecificClassType<ShortTypeNameMetadata> GetStaticClass();
    virtual ClassType cls() const override;

    // initialize class (since we have no automatic reflection in this project)
    static void RegisterType(TypeSystem& typeSystem);

private:
    const char* m_shortName;
};

//--

/// order of test for class-based tests (common, moved here)
class BM_CORE_OBJECT_API TestOrderMetadata : public IMetadata
{
public:
    INLINE TestOrderMetadata()
        : m_order(-1)
    {}

    INLINE TestOrderMetadata& order(int val)
    {
        m_order = val;
        return *this;
    }

    INLINE int order() const
    {
        return m_order;
    }

    //--

    static SpecificClassType<TestOrderMetadata> GetStaticClass();
    virtual ClassType cls() const override;

    // initialize class (since we have no automatic reflection in this project)
    static void RegisterType(TypeSystem& typeSystem);

    //--

private:
    int m_order;
};

//--

// general metadata to specify class internal "version", used to indicate major implementation changes that may require regenerating cached data
class BM_CORE_OBJECT_API ClassVersionMetadata : public IMetadata
{
public:
    INLINE ClassVersionMetadata()
    {}

    INLINE uint32_t version() const { return m_version; }
    INLINE void version(uint32_t version) { m_version = version; }

    //--

    static SpecificClassType<ClassVersionMetadata> GetStaticClass();
    virtual ClassType cls() const override;

    // initialize class (since we have no automatic reflection in this project)
    static void RegisterType(TypeSystem& typeSystem);

    //--

private:
    uint32_t m_version = 0;
};

END_INFERNO_NAMESPACE()
