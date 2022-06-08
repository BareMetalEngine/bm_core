/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: reflection #]
***/

#pragma once

#include "reflectionTypeName.h"
#include "reflectionPropertyBuilder.h"
#include "reflectionFunctionBuilder.h"

BEGIN_INFERNO_NAMESPACE()

class PropertyBuilder;
class FunctionBuilder;
class InterfaceBuilder;
class ClassBuilder;
class CustomTypeBuilder;

// trait builder
class BM_CORE_OBJECT_API TypeTraitBuilder : public MainPoolData<NoCopy>
{
public:
    TypeTraitBuilder();

    INLINE TypeTraitBuilder& zeroInitializationValid()
    {
        m_traitZeroInit = true;
        return *this;
    }

    INLINE TypeTraitBuilder& noConstructor()
    {
        m_traitNoConstructor = true;
        return *this;
    }

    INLINE TypeTraitBuilder& noDestructor()
    {
        m_traitNoDestructor = true;
        return *this;
    }

    INLINE TypeTraitBuilder& fastCopyCompare()
    {
        m_traitFastCopyCompare = true;
        return *this;
    }

private:
    bool m_traitZeroInit;
    bool m_traitNoConstructor;
    bool m_traitNoDestructor;
    bool m_traitFastCopyCompare;

    void apply(TypeRuntimeTraits& traits);

    friend class ClassBuilder;
    friend class CustomTypeBuilder;
};

// helper class that can add stuff to the class type
class BM_CORE_OBJECT_API ClassBuilder : public MainPoolData<NoCopy>
{    
public:
    ClassBuilder(NativeClass* classPtr);
    ~ClassBuilder();

    // apply changes to target class
    // this atomically sets up the base class and the properties
    void submit(); 

    // set current property category
    void category(const char* category);

    // create a builder for a class property 
    PropertyBuilder& addProperty(const char* rawName, Type type, uint32_t dataOffset);

    // create metadata
    IMetadata& addMetadata(ClassType classType);

    // create a native function builder
    void addNativeFunction(const char* rawName, const FunctionSignature& sig, const FunctionPointer& ptr, TFunctionWrapperPtr wrapPtr);

    //---

    template< typename T >
    INLINE PropertyBuilder& addProperty(const char* rawName, T& ptr)
    {
        auto offset = range_cast<uint32_t>((uint64_t)&ptr);
        return addProperty(rawName, GetTypeObject<T>(), offset);
    }

    template< typename T >
    INLINE T& addMetadata()
    {
        static_assert(std::is_base_of<IMetadata, T>::value, "Not a metadata class");
        return static_cast<T&>(addMetadata(ClassID<T>()));
    }

    INLINE TypeTraitBuilder& addTrait()
    {
        return m_traits;
    }

    INLINE NativeClass& type()
    {
        return *m_classPtr;
    }

    void addOldName(const char* oldName);

private:
    NativeClass* m_classPtr;
    Array<PropertyBuilder> m_properties;
    Array<StringID> m_oldNames;
    StringBuf m_categoryName;
    TypeTraitBuilder m_traits;

    struct NativeFunctionInfo
    {
        StringID rawName;
        FunctionSignature sig;
        FunctionPointer ptr; 
        TFunctionWrapperPtr wrapPtr = nullptr;
    };

    Array<NativeFunctionInfo> m_nativeFunctions;
};

//--

// helper class that can add stuff to the custom type
class BM_CORE_OBJECT_API CustomTypeBuilder : public MainPoolData<NoCopy>
{
public:
    CustomTypeBuilder(CustomType* customType);
    ~CustomTypeBuilder();

    // apply changes to target class
    // this atomically sets up the base class and the properties
    void submit();

    //--

    INLINE TypeTraitBuilder& addTrait()
    {
        return m_traits;
    }

    INLINE CustomType& type()
    {
        return *m_type;
    }

    //--

private:
    CustomType* m_type;
    TypeTraitBuilder m_traits;
};

END_INFERNO_NAMESPACE()
