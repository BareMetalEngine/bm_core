/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/hashSet.h"
#include "bm/core/containers/include/inplaceArray.h"
#include "bm/core/containers/include/bitUtils.h"
#include "bm/core/system/include/guid.h"

#include "serializationStream.h"

BEGIN_INFERNO_NAMESPACE()

///---
/// 
/// all collected references for opcode serialization
struct BM_CORE_OBJECT_API SerializationWriterReferences : public MainPoolData<NoCopy>
{
    SerializationWriterReferences();
         
    HashSet<StringID> stringIds;
    HashSet<Type> types;
    HashSet<const Property*> properties;
    HashSet<SerializationResourceKey> resources;
    HashSet<AsyncFileBufferLoaderPtr> asyncBuffers;
	HashSet<ObjectPtr> allObjects;
};

///---

/// serialization opcode stream builder
class BM_CORE_OBJECT_API SerializationWriter : public MainPoolData<NoCopy>
{
public:
	SerializationWriter(SerializationStream& stream, SerializationStreamAllocator& allocator, SerializationWriterReferences& referenceCollector, HashSet<IObject*>& strongObjects);
    ~SerializationWriter();

    //--

    //! did we have errors ?
    INLINE bool errors() const { return m_errors; }

    //--

    //! write raw bytes to stream
    INLINE void writeData(const void* data, uint64_t size);

    //! write typed data
    template< typename T >
    INLINE void writeTypedData(const T& data);

    //--

    //INLINE void beginCompound(Type type);
    //INLINE void endCompound();

    //INLINE void beginArray(uint32_t count);
    //INLINE void endArray();

    INLINE void beginSkipBlock();
    INLINE void endSkipBlock();

    INLINE void writeStringID(StringID id);
    INLINE void writeType(Type t);
    INLINE void writeProperty(const Property* rttiProperty);
    INLINE void writePointer(const IObject* object, bool strong);
    INLINE void writeCompressedUint(uint32_t value);

    void writeResourceReference(const SerializationResourceKey& key);

    void writeInlinedBuffer(Buffer data);
    void writeAsyncBuffer(IAsyncFileBufferLoader* loader);

    //--



    //--

private:
    SerializationStream& m_stream;
    SerializationStreamAllocator& m_allocator;
    SerializationWriterReferences& m_references;
    HashSet<IObject*>& m_strongObjects;

    //InplaceArray<SerializationOpcode, 20> m_stack;
    InplaceArray<SerializationOpSkipHeader*, 20> m_skips;
    //InplaceArray<SerializationOpCompound*, 20> m_compounds;

    bool writingFailed();

    bool m_errors = false;
};
       
///---

template< typename T >
INLINE void SerializationWriter::writeTypedData(const T& data)
{
    if (sizeof(T) <= 255)
    {
		auto op = m_allocator.allocOpcodeWithData<SerializationOpDataBlock1>(sizeof(T));
		DEBUG_CHECK_RETURN(op || writingFailed());

		op->size = sizeof(T);

		auto* writePtr = (uint8_t*)op + sizeof(SerializationOpDataBlock1);
        *(T*)writePtr = data;
    }
	else if (sizeof(T) <= 65535)
	{
		auto op = m_allocator.allocOpcodeWithData<SerializationOpDataBlock2>(sizeof(T));
		DEBUG_CHECK_RETURN(op || writingFailed());

		op->size = sizeof(T);

		auto* writePtr = (uint8_t*)op + sizeof(SerializationOpDataBlock2);
		*(T*)writePtr = data;
	}
    else
    {
		auto op = m_allocator.allocOpcodeWithData<SerializationOpDataBlock4>(sizeof(T));
		DEBUG_CHECK_RETURN(op || writingFailed());

		op->size = sizeof(T);

		auto* writePtr = (uint8_t*)op + sizeof(SerializationOpDataBlock2);
		*(T*)writePtr = data;
    }
}

INLINE void SerializationWriter::writeData(const void* data, uint64_t size)
{
	if (size <= 255)
	{
		auto op = m_allocator.allocOpcodeWithData<SerializationOpDataBlock1>(size);
		DEBUG_CHECK_RETURN(op || writingFailed());

		op->size = size;

		auto* writePtr = (uint8_t*)op + sizeof(SerializationOpDataBlock1);
		memcpy(writePtr, data, size);
	}
	else if (size <= 65535)
	{
		auto op = m_allocator.allocOpcodeWithData<SerializationOpDataBlock2>(size);
		DEBUG_CHECK_RETURN(op || writingFailed());

		op->size = size;

		auto* writePtr = (uint8_t*)op + sizeof(SerializationOpDataBlock2);
		memcpy(writePtr, data, size);
	}
	else
	{
		auto op = m_allocator.allocOpcodeWithData<SerializationOpDataBlock4>(size);
		DEBUG_CHECK_RETURN(op || writingFailed());

		op->size = size;

		auto* writePtr = (uint8_t*)op + sizeof(SerializationOpDataBlock4);
		memcpy(writePtr, data, size);
	}	
}

INLINE void SerializationWriter::writeCompressedUint(uint32_t value)
{
	auto op = m_allocator.allocOpcode<SerializationOpDataAdaptiveNumber>();
	DEBUG_CHECK_RETURN(op || writingFailed());
    op->value = value;
}

/*INLINE void SerializationWriter::beginCompound(Type type)
{
    auto op = m_allocator.allocOpcode<SerializationOpCompound>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->numProperties = 0;
    op->type = type;

    m_compounds.pushBack(op);
    m_stack.pushBack(SerializationOpcode::Compound);
}

INLINE void SerializationWriter::endCompound()
{
    DEBUG_CHECK_RETURN_EX(!m_stack.empty() || writingFailed(), "Invalid opcode stack - binary steam is mallformed, must be fixed");
    DEBUG_CHECK_RETURN_EX(m_stack.back() == SerializationOpcode::Compound || writingFailed(), "No inside a compound - binary steam is mallformed, must be fixed");
    m_stack.popBack();
    DEBUG_CHECK_RETURN_EX(!m_compounds.empty() || writingFailed(), "No compounds on stack");
    m_compounds.popBack();

    m_allocator.allocOpcode<SerializationOpCompoundEnd>();
}

INLINE void SerializationWriter::beginArray(uint32_t count)
{
    auto op = m_allocator.allocOpcode<SerializationOpArray>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->count = count;

    m_stack.pushBack(SerializationOpcode::Array);
}

INLINE void SerializationWriter::endArray()
{
    DEBUG_CHECK_RETURN_EX(!m_stack.empty() || writingFailed(), "Invalid opcode stack - binary steam is mallformed, must be fixed");
    DEBUG_CHECK_RETURN_EX(m_stack.back() == SerializationOpcode::Array || writingFailed(), "No inside an array - binary steam is mallformed, must be fixed");
    m_stack.popBack();

    m_allocator.allocOpcode<SerializationOpArrayEnd>();
}*/

INLINE void SerializationWriter::beginSkipBlock()
{
    auto op = m_allocator.allocOpcode<SerializationOpSkipHeader>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->skipSizeValueSize = 0;
    op->skipOffset = 0;

    m_skips.pushBack(op);
}

INLINE void SerializationWriter::endSkipBlock()
{
    //DEBUG_CHECK_RETURN_EX(!m_stack.empty() || writingFailed(), "Invalid opcode stack - binary steam is mallformed, must be fixed");
    //DEBUG_CHECK_RETURN_EX(m_stack.back() == SerializationOpcode::SkipHeader || writingFailed(), "No inside a skip block - binary steam is mallformed, must be fixed");
    //m_stack.popBack();

    DEBUG_CHECK_RETURN_EX(!m_skips.empty() || writingFailed(), "No active skip block");

    auto header = m_skips.back(); // can be NULL in OOM condition
    m_skips.popBack();

    auto op = m_allocator.allocOpcode<SerializationOpSkipLabel>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->header = header;
}

INLINE void SerializationWriter::writeStringID(StringID id)
{
    if (id)
        m_references.stringIds.insert(id);

    auto op = m_allocator.allocOpcode<SerializationOpDataName>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->shared.name = id;
}

INLINE void SerializationWriter::writeType(Type t)
{
    if (t)
        m_references.types.insert(t);

    auto op = m_allocator.allocOpcode<SerializationOpDataTypeRef>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->shared.type = t.ptr();
}

INLINE void SerializationWriter::writeProperty(const Property* rttiProperty)
{
    //DEBUG_CHECK_RETURN_EX(!m_stack.empty() || writingFailed(), "Property must be inside a compound");
    //DEBUG_CHECK_RETURN_EX(m_stack.back() == SerializationOpcode::Compound || writingFailed(), "Property must be directly inside a compound");
    //DEBUG_CHECK_RETURN_EX(!m_compounds.empty() || writingFailed(), "Property must be inside a compound");

    //if (auto* compound = m_compounds.back())
      //  compound->numProperties += 1;

    if (rttiProperty)
        m_references.properties.insert(rttiProperty);

    auto op = m_allocator.allocOpcode<SerializationOpProperty>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->shared.prop = rttiProperty;
}

INLINE void SerializationWriter::writePointer(const IObject* object, bool strong)
{
    if (object)
    {
        m_references.allObjects.insert(AddRef(object));

        if (strong)
            m_strongObjects.insert(AddRef(object));
    }

    auto op = m_allocator.allocOpcode<SerializationOpDataObjectPointer>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    op->shared.object = object;
    op->strong = strong;
}

//--

END_INFERNO_NAMESPACE()
