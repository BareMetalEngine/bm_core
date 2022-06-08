/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiType.h"
#include "rttiProperty.h"
#include "object.h"

#include "serializationStream.h"
#include "bm/core/memory/include/localAllocator.h"
#include "bm/core/containers/include/bitUtils.h"

BEGIN_INFERNO_NAMESPACE()

//--

const char* SerializationOpcodeName(SerializationOpcode op)
{
    switch (op)
    {
#define SERIALIZATION_OPCODE(x) case SerializationOpcode::x: return #x; 
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE
    }
    return "UnknownOpcode";
}

uint64_t SerializationOpBase::CalcSize(const SerializationOpBase* op)
{
    switch (op->op)
    {
#define SERIALIZATION_OPCODE(x) case SerializationOpcode::x: return SerializationOp##x::CalcAdditionalSize((const SerializationOp##x*)op) + sizeof(SerializationOp##x);
#include "serializationStream.inl"
#undef SERIALIZATION_OPCODE
    }

    ASSERT(!"Unknown opcode");
    return 0;
}

//--
/*
uint64_t SerializationOpDataRaw::dataSize() const
{
    const auto* ptr = (const uint8_t*)this + 1;
    uint8_t size = 0;
    return ReadAdaptiveNumber4(ptr, size);
}

const void* SerializationOpDataRaw::data() const
{
    const auto* ptr = (const uint8_t*)this + 1;
    uint8_t size = 0;
    ReadAdaptiveNumber4(ptr, size);
    ptr += size;
    return ptr;
}*/

//--

ISerializationStreamVisitor::~ISerializationStreamVisitor()
{}

//--

SerializationStreamIterator::SerializationStreamIterator(const SerializationStream* stream /*= nullptr*/)
    : m_cur(stream->m_firstOpcode)
    , m_end(stream->m_lastOpcode)
{
}

SerializationStreamIterator::SerializationStreamIterator(const uint8_t* firstOpcode, const uint8_t* lastOpcode)
    : m_cur(firstOpcode)
    , m_end(lastOpcode)
{
}

SerializationStreamIterator::SerializationStreamIterator(const SerializationStreamIterator& other) = default;
SerializationStreamIterator& SerializationStreamIterator::operator=(const SerializationStreamIterator& other) = default;

void SerializationStreamIterator::advance()
{
    // already at the end
    if (m_cur != m_end)
    {
		const auto* op = (const SerializationOpNextPage*)m_cur;

        if (op->op != SerializationOpcode::NextPage)
        {
			// advance over current
            const auto size = SerializationOpBase::CalcSize((const SerializationOpBase*)m_cur);
            m_cur += size;
        }
        else
        {
            // jump to new location
            m_cur = (const uint8_t*)op->nextPage;
        }
    }
}

//--

SerializationStreamAllocator::SerializationStreamAllocator(LocalAllocator& mem)
    : m_allocator(mem)
{
    allocPage(OPCODE_PAGE_SIZE);
}

void* SerializationStreamAllocator::allocMore(uint64_t size)
{
    if (!allocPage(size))
        return nullptr;

	auto* ptr = m_writePtr;
	m_writePtr += size;
	m_totalBlocksAllocated += 1;
	m_totalMemoryAllocated += size;

    ASSERT(ptr + size <= m_writeEnd);
    return ptr;
}

bool SerializationStreamAllocator::allocPage(uint64_t requiredSize)
{
    // out of memory
    if (m_outOfMemory)
        return false;

	// open a new one
	const auto pageSize = NextPowerOf2(std::max<uint64_t>(OPCODE_PAGE_SIZE, requiredSize));
	void* memory = m_allocator.alloc(pageSize, 4);

	// internal out of memory :(
	if (nullptr == memory)
	{
		TRACE_ERROR("OutOfMemory when allocting additional page for serialization stream");
		m_outOfMemory = true;
        m_writePtr = nullptr;
        m_writeEnd = nullptr;
		return false;
	}

    // write link
    if (m_writePtr)
    {
        auto* op = (SerializationOpNextPage*)m_writePtr;
        op->op = SerializationOpNextPage::OP;
        op->nextPage = memory;
    }

	// setup page header
    m_writePtr = (uint8_t*)memory;
    m_writeEnd = m_writePtr + pageSize - 32;
    m_writePtr[0] = 0xCC;

	// ready to write
	return true;
}

//--

SerializationStream::SerializationStream()
{
}

SerializationStream::~SerializationStream()
{
	for (auto* op : m_inlinedBuffers)
		op->~SerializationOpDataInlineBuffer();

	for (auto* op : m_resourceReferences)
		op->~SerializationOpDataResourceRef();
}
 

//--

SerializationStreamIterator SerializationStream::opcodes() const
{
    return SerializationStreamIterator(this);
}

void SerializationStream::dispatch(ISerializationStreamVisitor& dispatcher) const
{
    for (SerializationStreamIterator it(this); it; ++it)
        dispatcher.dispatchOpcode(*it);
}

class OpcodeExtraInfoPrinter : public ISerializationStreamVisitor
{
public:
    OpcodeExtraInfoPrinter(IFormatStream& f)
        : m_stream(f)
    {}

    virtual void processOpcode(const SerializationOpCompound& op) override
    {
        m_stream.appendf(" Type='{}'", op.type);
    }

    virtual void processOpcode(const SerializationOpArray& op) override
    {
        m_stream.appendf(" Size={}", op.count);
    }

    virtual void processOpcode(const SerializationOpProperty& op) override
    {
        m_stream.appendf(" Name='{}', Type={}", op.shared.prop->name(), op.shared.prop->type());
    }

    virtual void processOpcode(const SerializationOpDataName& op) override
    {
        m_stream.appendf(" Name='{}'", op.shared.name);
    }

    virtual void processOpcode(const SerializationOpDataTypeRef& op) override
    {
        m_stream.appendf(" Type='{}'", op.shared.type);
    }

    virtual void processOpcode(const SerializationOpDataResourceRef& op) override
    {
        if (op.shared.key.id)
            m_stream.appendf(" Id='{}' Cls='{}'", op.shared.key.id, op.shared.key.className);
        else
            m_stream.appendf(" NoResource");
    }

    virtual void processOpcode(const SerializationOpDataObjectPointer& op) override
    {
        if (op.shared.object)
            m_stream.appendf(" Type='{}', Pointer=0x{}", op.shared.object->cls().name(), Hex((uint64_t)op.shared.object));
        else
            m_stream.appendf(" NULL");
    }

    virtual void processOpcode(const SerializationOpDataBlock1& op) override
    {
        m_stream.appendf(" Size='{}' ", op.size);
    }

	virtual void processOpcode(const SerializationOpDataAdaptiveNumber& op) override
	{
		m_stream.appendf(" Number='{}' ", op.value);
	}

	virtual void processOpcode(const SerializationOpDataBlock2& op) override
	{
		m_stream.appendf(" Size='{}' ", op.size);
	}

	virtual void processOpcode(const SerializationOpDataBlock4& op) override
	{
		m_stream.appendf(" Size='{}' ", op.size);
	}

private:
    IFormatStream& m_stream;
};

void SerializationStream::print(IFormatStream& f) const
{
    OpcodeExtraInfoPrinter printer(f);

    for (SerializationStreamIterator it(this); it; ++it)
    {
        const auto* op = *it;
        f.append(SerializationOpcodeName(op->op));
        printer.dispatchOpcode(op);
        f.append("\n");
    }
}

//--

END_INFERNO_NAMESPACE()
