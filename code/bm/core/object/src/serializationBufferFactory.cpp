/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "serializationFileTables.h"
#include "serializationBufferFactory.h"

BEGIN_INFERNO_NAMESPACE()

//--

ISerializationBufferFactory::~ISerializationBufferFactory()
{}

//--

#if 0
SerializationBufferFactory_AbsoluteFile::SerializationBufferFactory_AbsoluteFile(const StringBuf& absolutePath, const SerializationBinaryFileTables* fileTables, TimeStamp fileTimestamp)
    : m_absolutePath(absolutePath)
    , m_fileTables(fileTables)
    , m_fileTimestamp(fileTimestamp)
{}

AsyncFileBufferLoaderPtr SerializationBufferFactory_AbsoluteFile::createAsyncBufferLoader(uint64_t bufferID) const
{
    const auto numBuffers = m_fileTables->chunkCount(SerializationBinaryFileTables::ChunkType::Buffers);
    DEBUG_CHECK_RETURN_EX_V(bufferID > 0 && bufferID <= numBuffers, "Invalid buffer ID", nullptr);

    const auto& bufferInfo = m_fileTables->bufferTable()[bufferID - 1];

    /*TaskList tl;
    tl.maxConcurency(1);
    tl.add("Prepare") << []();
    tl.barrier();
    tl.add("Compute") << []();
    tl.barrier();
    tl.runAsync();
    tl.runAndWait(Spin);
    tl.runAndWait();
    tl.runAndWait(ParticipateLocal);
    tl.runAndWait(ParticipateTree);
    tl.runAndWait(ParticipateGlobal);*/

    return nullptr;
}
#endif

//--

END_INFERNO_NAMESPACE()

