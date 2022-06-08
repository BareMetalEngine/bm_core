/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types\class #]
***/

#include "build.h"
#include "rttiClassType.h"
#include "rttiNativeClassType.h"

BEGIN_INFERNO_NAMESPACE()

//--

NativeClass::NativeClass(const char* name, uint32_t size, uint32_t alignment, uint64_t nativeHash)
    : IClassType(StringID(name), size, alignment)
{
    DEBUG_CHECK_EX(nativeHash != 0, "Native class with no native type hash");
    m_traits.nativeHash = nativeHash;
}

bool NativeClass::isAbstract() const
{
    DEBUG_CHECK_EX((!funcConstruct && !funcDestruct) || (funcConstruct && funcDestruct), TempString("Both constructor and destructor should be specified or neither at class '{}'", name()));
    return !funcConstruct || !funcDestruct;
}

bool NativeClass::isConstructible() const
{
    return (funcConstruct != nullptr) && (funcDestruct != nullptr);
}

const void* NativeClass::defaultObject() const
{
    const void* currentDefault = m_defaultObject.load();
    if (!currentDefault)
    {
        currentDefault = createDefaultObject();
        m_defaultObject.exchange(currentDefault); // note: we don't care if we leak a default object, they are all the same and read only
    }
    return currentDefault;
}

void NativeClass::destroyDefaultObject() const
{
    if (auto* ptr = (void*)m_defaultObject.exchange(nullptr))
    {
        EnterDefaultObjectCreation();
        if (funcDestruct)
            funcDestruct(ptr);
        LeaveDefaultObjectCreation();
    }
}

const void* NativeClass::createDefaultObject() const
{
    void* mem = Memory::AllocateBlock(size(), alignment(), name().c_str());
    memzero(mem, size());

    EnterDefaultObjectCreation();
    if (funcConstruct)
        funcConstruct(mem);
    LeaveDefaultObjectCreation();

    return mem;
}

void NativeClass::construct(void* object) const
{
    auto memSize = size();

    if (funcConstruct)
    {
        funcConstruct(object);
    }
    else
    {
        //ASSERT_EX(!is<IObject>(), "All object should have constructors");
        memzero(object, memSize); // we don't have a constructor function, just zero initialize the memory
    }
}
    
void NativeClass::destruct(void* object) const
{
    if (funcDestruct)
        funcDestruct(object);
}

bool NativeClass::compare(const void* data1, const void* data2) const
{
    if (funcComare)
        return funcComare(data1, data2);
    else
        return IClassType::compare(data1, data2);
}

void NativeClass::copy(void* dest, const void* src) const
{
    if (funcCopyAssign)
        return funcCopyAssign(dest, src);
    else
        IClassType::copy(dest, src);
}

void NativeClass::writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const
{
    if (funcWriteBinary)
        funcWriteBinary(typeContext, file, data, defaultData);
    else
        IClassType::writeBinary(typeContext, file, data, defaultData);
}

void NativeClass::readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const
{
    if (funcReadBinary)
        funcReadBinary(typeContext, file, data);
    else
        IClassType::readBinary(typeContext, file, data);
}

//--

END_INFERNO_NAMESPACE()
