/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti #]
***/

#pragma once

#include "bm/core/containers/include/stringID.h"
#include "bm/core/containers/include/array.h"
#include "bm/core/containers/include/hashMap.h"
#include "bm/core/containers/include/inplaceArray.h"

#include "rttiTypeRef.h"

#include <functional>

BEGIN_INFERNO_NAMESPACE()

//--

/// RTTI SYSTEM interface
class BM_CORE_OBJECT_API TypeSystem : public ISingleton
{
public:
    /// Find type by name (any type)
    /// NOTE: array and pointer types may be created if the name fits
    Type findType(StringID typeName);

    /// Find a class only type definition (a little bit faster than general FindType)
    ClassType findClass(StringID name);

    /// Find an enum only type definition (a little bit faster than general FindType)
    const EnumType* findEnum(StringID name);

    /// Find class property using the property hash (much faster than looking it up manually)
    const Property* findProperty(uint64_t propertyHash);

    /// Find global function
    const IFunction* findGlobalFunction(StringID name);

    /// Enumerate classes derived from given base class
    typedef std::function<bool(ClassType )> TClassFilter;
    typedef Array<ClassType > TClassList;
    void enumClasses(ClassType baseClass, TClassList& outClasses, const TClassFilter& filter = TClassFilter(), bool allowAbstract = false);

    /// Enumerate classes directly derived from given IClassType
    void enumDerivedClasses(ClassType baseClass, TClassList& outClasses);

    /// Enumerate all base types (non dynamically created) 
    void enumBaseTypes(Array<Type>& outAllTypes);

    /// Enumerate all global functions
    typedef Array<const IFunction*> TGlobalFunctionList;
    void enumGlobalFunctions(TGlobalFunctionList& outAllGlobalFunctions);

    //---

    /// Register type in the RTTI system
    void registerType(Type type);

    /// Register alternative name for type (usually old)
    void registerAlternativeTypeName(Type type, StringID alternativeName);

    /// Register class property
    void registerProperty(const Property* prop);

    /// Register a global function
    void registerGlobalFunction(const IFunction* func);

    //--

    /// Register a Dynamic Type Creator. Dynamic type are usually a type that composition of multiple type. Like DynArray or THandle.
    typedef Type (*TDynamicTypeCreationFunction)(StringParser& typeNameString, TypeSystem& typeSystem);
    void registerDynamicTypeCreator(const char* keyword, TDynamicTypeCreationFunction creatorFunction);

    //---

    /// Given a native type hash (from typeid().hash_code()) find a matching RTTI type object
    /// NOTE: use std::remove_cv before calculating the hash
    Type mapNativeType(uint64_t hashCode) const;

    /// Given a native type hash (from typeid().hash_code()) find a matching RTTI type name
    /// NOTE: use std::remove_cv before calculating the hash
    StringID mapNativeTypeName(uint64_t hashCode) const;

    //---

    template< typename T >
    INLINE TClassList enumChildClasses(const TClassFilter& filter = TClassFilter(), bool allowAbstract = false)
    {
        TClassList ret;
        enumClasses(T::GetStaticClass(), ret, filter, allowAbstract);
        return ret;
    }

    /// Enumerate classes derived from given base class
    template< typename T >
    INLINE void enumClasses(Array< SpecificClassType<T> >& outClasses, const TClassFilter& filter = TClassFilter(), bool allowAbstract = false)
    {
        enumClasses(T::GetStaticClass(), (Array<ClassType>&)outClasses, filter, allowAbstract);
    }

    /// Enumerate classes derived from given base class
    template< typename T >
    INLINE void enumClasses(ClassType cls, Array<SpecificClassType<T>>& outClasses, const TClassFilter& filter = TClassFilter(), bool allowAbstract = false)
    {
        if (cls.is<T>())
            enumClasses(cls, (Array<ClassType>&)outClasses, filter, allowAbstract);
    }

    //---

    static TypeSystem& GetInstance();

    //--

    /// find a class by name that can be used to create object
    ClassType findFactoryClass(StringID name, ClassType requiredBaseClass);

protected:
    TypeSystem();

    static TypeSystem* CreateTypeSystem(bool& outInit);

    void init();
    virtual void deinit() override;

    typedef HashMap<StringID, Type>  TTypeMap;
    typedef Array<Type>  TTypeList;
    typedef HashMap<uint64_t, const Property*>  TPropertyMap;
    typedef HashMap<uint64_t, Type>  TNativeTypeMap;
    typedef HashMap<StringID, const IFunction*>  TGlobalFunctionMap;
    typedef Array<ClassType>  TStorageClassList;
    typedef Array<std::pair< const char*, TDynamicTypeCreationFunction > > TDynamicTypeCreators;
    typedef HashMap<ClassType, TClassList> TChildClasses;

    Mutex m_typesLock;
    TTypeMap m_types;
    TTypeMap m_alternativeTypes;
    TTypeList m_typeList;
    TStorageClassList m_classes;
    
    Mutex m_nativeMapLock;
    TNativeTypeMap m_nativeMap;

    Mutex m_propertiesLock;
    TPropertyMap m_properties;

    Mutex m_globalFunctionsLock;
    TGlobalFunctionList m_globalFunctions;
    TGlobalFunctionMap m_globalFunctionsMap;

    TDynamicTypeCreators m_typeCreators;

    TChildClasses m_childClasses;

    bool m_fullyInitialized;

    Type createDynamicType_NoLock(const char* typeName);
};

//--

/// copy value from one place to other, does type conversion when needed
/// returns false if type conversion was not possible
/// supported conversions:
///   same type to same type always works
///   numerical types (int8,16,32,64, uint8,16,32,64)
///   real types: float -> double, double -> float
///   handle types (via rtti_cast)
///   resource refs (of matching classes, ie Ref<StaticTexture> -> Ref<ITexture>, actual class is only checked if resource is loaded)
///   async resource refs (of matching classes, ie AsyncRef<StaticTexture> -> AsyncRef<ITexture>, downcasting is not possible as actual class is not known)
///   stringBuf <-> stringId
///   enum <-> stringID (if enum does not have a option with that name then we fail the conversion)
///   enum <-> (u)int64 (raw value)
///   bitfield <-> (u)int64 (raw value)
///   classTypes <-> specficClassTypes
///   SIMPLE string <-> type conversions (numbers, boolean)
extern BM_CORE_OBJECT_API bool ConvertData(const void* srcData, Type srcType, void* destData, Type destType);
extern BM_CORE_OBJECT_API bool ConvertData(const TypedMemory& src, const TypedMemory& dest);

//--

/// this is one of the major singletons, it deserves a global access
using RTTI = TypeSystem;

END_INFERNO_NAMESPACE()