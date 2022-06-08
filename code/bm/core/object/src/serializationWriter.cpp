/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: serialization #]
***/

#include "build.h"
#include "rttiType.h"
#include "rttiProperty.h"

#include "asyncBuffer.h"
#include "serializationStream.h"
#include "serializationWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

SerializationWriterReferences::SerializationWriterReferences()
{
    stringIds.reserve(1024);
    allObjects.reserve(1024);
    properties.reserve(1024);
    resources.reserve(1024);
}
        
//--

SerializationWriter::SerializationWriter(SerializationStream& stream, SerializationStreamAllocator& allocator, SerializationWriterReferences& referenceCollector, HashSet<IObject*>& strongObjects)
    : m_stream(stream)
    , m_allocator(allocator)
    , m_references(referenceCollector)
    , m_strongObjects(strongObjects)
{
    m_stream.m_firstOpcode = m_allocator.pointer();
    m_stream.m_lastOpcode = m_allocator.pointer();
}

SerializationWriter::~SerializationWriter()
{
    m_stream.m_lastOpcode = m_allocator.pointer();
}       
    
bool SerializationWriter::writingFailed()
{
    if (!m_errors)
    {
        TRACE_WARNING("Serialization writer has failed");
        m_errors = true;
    }

    return false;
}

void SerializationWriter::writeResourceReference(const SerializationResourceKey& key)
{
    if (key.id)
    {
        m_references.resources.insert(key);

        if (auto op = m_allocator.allocOpcode<SerializationOpDataResourceRef>())
        {
            op->shared.key = key;
            m_stream.m_resourceReferences.pushBack(op);
        }
    }
    else
    {
        m_allocator.allocOpcode<SerializationOpDataResourceRef>();
    }
}

void SerializationWriter::writeInlinedBuffer(Buffer data)
{
    auto op = m_allocator.allocOpcode<SerializationOpDataInlineBuffer>();
    DEBUG_CHECK_RETURN(op || writingFailed());

    memzero(&op->data, sizeof(op->data));
    memzero(&op->asyncLoader, sizeof(op->asyncLoader));    
    op->data = data;

    m_stream.m_inlinedBuffers.pushBack(op);
}

void SerializationWriter::writeAsyncBuffer(IAsyncFileBufferLoader* loader)
{
    auto op = m_allocator.allocOpcode<SerializationOpDataInlineBuffer>();
    DEBUG_CHECK_RETURN(op || writingFailed());

	memzero(&op->data, sizeof(op->data));
	memzero(&op->asyncLoader, sizeof(op->asyncLoader));
    op->asyncLoader = AddRef(loader);

    m_references.asyncBuffers.insert(AddRef(loader));
    m_stream.m_inlinedBuffers.pushBack(op);
}

//--

END_INFERNO_NAMESPACE()
