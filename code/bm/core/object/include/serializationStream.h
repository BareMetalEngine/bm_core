/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/hashSet.h"

BEGIN_INFERNO_NAMESPACE()

///---

class SerializationStream;

typedef uint16_t MappedNameIndex;
typedef uint16_t MappedTypeIndex;
typedef uint16_t MappedPropertyIndex;
typedef uint16_t MappedPathIndex;
typedef uint32_t MappedObjectIndex; // yup, it happened 64K+ objects in one files
typedef uint16_t MappedBufferIndex;

/// serialization resource key
struct SerializationResourceKey
{
	GUID id;
	StringID className;

	INLINE SerializationResourceKey() = default;

	INLINE bool operator==(const SerializationResourceKey& other) const { return (id == other.id) && (className == other.className); }
	INLINE bool operator!=(const SerializationResourceKey& other) const { return !operator==(other); }

	static uint32_t CalcHash(const SerializationResourceKey& key)
	{
		return CRC32() << key.id.data()[3] << key.id.data()[2] << key.className.index();
	}
};

///---

enum class SerializationOpcode : uint8_t
{
#define SERIALIZATION_OPCODE(x) x,
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE
};

///---

// get opcode printable name
extern BM_CORE_OBJECT_API const char* SerializationOpcodeName(SerializationOpcode op);

///---

#pragma pack(push)
#pragma pack(1)

// NOTE: the destructor of the Op structures are NEVER CALLED, if you store something there that requires destruction you will leak memory
// Don't worry, you will learn about it soon, really soon
struct SerializationOpBase
{
    SerializationOpcode op = SerializationOpcode::Nop;

    BM_CORE_OBJECT_API static uint64_t CalcSize(const SerializationOpBase* op);
    INLINE static uint32_t CalcAdditionalSize(const SerializationOpBase* op) { return 0; }
};

template< SerializationOpcode opcode, typename SelfT >
struct SerializationOpBaseT : public SerializationOpBase
{
    static const auto OP = opcode;

    INLINE void setup()
    {
        op = OP;
    }
};

#define SERIALIZATION_OPCODE(x) struct SerializationOp##x;
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE

#define STREAM_OPCODE_DATA(op_) struct SerializationOp##op_ : public SerializationOpBaseT<SerializationOpcode::op_, SerializationOp##op_>

STREAM_OPCODE_DATA(Nop)
{
};

STREAM_OPCODE_DATA(Compound)
{
    Type type;
    uint16_t numProperties = 0;
};

STREAM_OPCODE_DATA(CompoundEnd)
{
};

STREAM_OPCODE_DATA(Array)
{
    uint32_t count = 0;
};

STREAM_OPCODE_DATA(ArrayEnd)
{
};

STREAM_OPCODE_DATA(Property)
{
    union
    {
        const Property* prop = nullptr;
        uint32_t index;
    } shared;
};

STREAM_OPCODE_DATA(SkipHeader)
{
    //SerializationOpSkipLabel* label = nullptr;
    uint8_t skipSizeValueSize = 0;
    uint32_t skipOffset = 0;
};

STREAM_OPCODE_DATA(SkipLabel)
{
    SerializationOpSkipHeader* header = nullptr;
};

STREAM_OPCODE_DATA(DataAdaptiveNumber)
{
    uint32_t value = 0;
};

STREAM_OPCODE_DATA(DataBlock1)
{
    uint8_t size = 0;

    INLINE static uint32_t CalcAdditionalSize(const SerializationOpDataBlock1 * op) { return op->size; }
};

STREAM_OPCODE_DATA(DataBlock2)
{
    uint16_t size = 0;

    INLINE static uint32_t CalcAdditionalSize(const SerializationOpDataBlock2* op) { return op->size; }
};

STREAM_OPCODE_DATA(DataBlock4)
{
    uint32_t size = 0;

    INLINE static uint32_t CalcAdditionalSize(const SerializationOpDataBlock4* op) { return op->size; }
};

STREAM_OPCODE_DATA(DataTypeRef)
{
    union
    {
        const IType* type = nullptr;
        uint32_t index;
    } shared;
};

STREAM_OPCODE_DATA(DataName)
{
    union
    {
        StringID name;
        uint32_t index;
    } shared;
};

STREAM_OPCODE_DATA(DataInlineBuffer)
{
    AsyncFileBufferLoaderPtr asyncLoader;
    Buffer data; // destroyed explicitly, always raw data
};

STREAM_OPCODE_DATA(DataObjectPointer)
{
    union
    {
        const IObject* object = nullptr; // NOT add-reffed (reference is kept elsewhere)
        uint32_t index;
    } shared;

    bool strong = false;
};

STREAM_OPCODE_DATA(DataResourceRef)
{
    union
    {
        SerializationResourceKey key;
        uint32_t index;
    } shared;
};

STREAM_OPCODE_DATA(DataAsyncFileBuffer)
{
    // TODO!
};

STREAM_OPCODE_DATA(NextPage)
{
    void* nextPage = nullptr;
};

#pragma pack(pop)

#define SERIALIZATION_OPCODE(x) static_assert(sizeof(SerializationOp##x) != 0, "Opcode structure not defined");
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE

///---

/// opcode dispatcher, allows to visit every opcode, virtual-based so used mostly for stats/dumping 
class BM_CORE_OBJECT_API ISerializationStreamVisitor : public MainPoolData<NoCopy>
{
public:
    virtual ~ISerializationStreamVisitor();

#define SERIALIZATION_OPCODE(x) virtual void processOpcode(const SerializationOp##x& op) {};
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE

    void dispatchOpcode(const SerializationOpBase* ptr)
    {
        switch (ptr->op)
        {
#define SERIALIZATION_OPCODE(x) case SerializationOpcode::x: processOpcode(*(const SerializationOp##x*)ptr); break;
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE
        default: ASSERT(!"Invalid stream opcode");
        }
    }
};

///---

/// opcode iterator
class BM_CORE_OBJECT_API SerializationStreamIterator
{
public:
    SerializationStreamIterator(const SerializationStream* stream = nullptr);
    SerializationStreamIterator(const uint8_t* firstOpcode, const uint8_t* lastOpcode);
    SerializationStreamIterator(const SerializationStreamIterator& other);
    SerializationStreamIterator& operator=(const SerializationStreamIterator& other);

    INLINE const SerializationOpBase* operator->() const { return (const SerializationOpBase*)m_cur; }
    INLINE const SerializationOpBase* operator*() const { return (const SerializationOpBase*)m_cur; }

    INLINE operator bool() const { return m_cur != m_end; }

    INLINE void operator++() { advance(); }
    INLINE void operator++(int) { advance(); }

private:
    const uint8_t* m_cur = nullptr;
    const uint8_t* m_end = nullptr;

    void advance();
};

///---

/// allocator for serialization stream
class BM_CORE_OBJECT_API SerializationStreamAllocator : public MainPoolData<NoCopy>
{
public:
    SerializationStreamAllocator(LocalAllocator& mem);

    //! get current opcode pointer
    INLINE const uint8_t* pointer() const { return m_writePtr; }

    //! number of blocks allocator
    INLINE uint32_t totalBlocksAllocated() const { return m_totalBlocksAllocated; }

    //! number of bytes allocator
    INLINE uint64_t totalBytesAllocator() const { return m_totalMemoryAllocated; }

    //! did we get out of memory ?
    INLINE bool outOfMemory() const { return m_outOfMemory; }

	//! allocate opcode with no extra data
	template< typename T >
	INLINE T* allocOpcode()
	{
		auto* op = (T*)allocInternal<sizeof(T)>();
		op->setup();
		return op;
	}

	//! allocate opcode with extra data
	template< typename T >
	INLINE T* allocOpcodeWithData(uint32_t size)
	{
        auto* op = (T*)allocInternal(sizeof(T) + size);
		op->setup();
		return op;
	}
    
private:
	static const uint32_t OPCODE_PAGE_SIZE = 8192;

    uint8_t* m_writePtr = nullptr;
    uint8_t* m_writeEnd = nullptr; // has space for the "NextPage"

    uint32_t m_totalBlocksAllocated = 0;
    uint64_t m_totalMemoryAllocated = 0;

    LocalAllocator& m_allocator;
    bool m_outOfMemory = false;

	bool allocPage(uint64_t requiredSize);
	void* allocMore(uint64_t size);

	template< uint32_t Size >
    INLINE void* allocInternal()
    {
		if (m_writePtr + Size <= m_writeEnd)
		{
			auto* ptr = m_writePtr;
			m_writePtr += Size;
#ifdef BUILD_DEBUG
			m_totalBlocksAllocated += 1;
			m_totalMemoryAllocated += Size;
#endif
			return ptr;
		}

        return allocMore(Size);
    }

	INLINE void* allocInternal(uint32_t size)
	{
		if (m_writePtr + size <= m_writeEnd)
		{
			auto* ptr = m_writePtr;
			m_writePtr += size;
#ifdef BUILD_DEBUG
			m_totalBlocksAllocated += 1;
			m_totalMemoryAllocated += size;
#endif
			return ptr;
		}

		return allocMore(size);
	}
};

///---

/// serialization opcode stream
class BM_CORE_OBJECT_API SerializationStream : public MainPoolData<NoCopy>
{
public:
    SerializationStream();
    ~SerializationStream();

    //--

    // get opcode stream for iteration
    SerializationStreamIterator opcodes() const;

	//--

    // process all opcodes with a visitor
    void dispatch(ISerializationStreamVisitor& dispatcher) const;

    /// dump stream to text (debug only)
    void print(IFormatStream& f) const;

	//--

private:
    const uint8_t* m_firstOpcode = nullptr;
    const uint8_t* m_lastOpcode = nullptr;

	Array<SerializationOpDataInlineBuffer*> m_inlinedBuffers; // all stored inlined buffers
	Array<SerializationOpDataResourceRef*> m_resourceReferences; // all stored inlined buffers

    friend class SerializationStreamIterator;
    friend class SerializationWriter;
};

///---

END_INFERNO_NAMESPACE()
