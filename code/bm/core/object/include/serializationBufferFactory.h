/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

//--

// helper class that can handle deferred loading of async buffer content
class BM_CORE_OBJECT_API ISerializationBufferFactory : public MainPoolData<NoCopy>
{
public:
	virtual ~ISerializationBufferFactory();

    struct BufferInfo
    {
        uint64_t crc = 0;
        uint64_t fileOffset = 0;
        uint32_t compressedSize = 0;
        uint32_t uncompressedSize = 0;
        CompressionType compressionType = CompressionType::Uncompressed;
    };

	/// create async buffer loader for a buffer with given CRC
	virtual AsyncFileBufferLoaderPtr createAsyncBufferLoader(const BufferInfo& info) const = 0;
};

#if 0
//--

// helper class that can handle deferred loading of async file from "file-like" sources, all this factory requires to function is a way to restore file access
class BM_CORE_OBJECT_API ISerializationBufferFactory_FileBasedPersistentStorage : public ISerializationBufferFactory
{
public:
    ISerializationBufferFactory_FileBasedPersistentStorage();

    /// implementation that restores access to file

};

//--

// absolute file implementation of the buffer factory
class BM_CORE_OBJECT_API SerializationBufferFactory_AbsoluteFile : public ISerializationBufferFactory
{
public:
    SerializationBufferFactory_AbsoluteFile(const StringBuf& absolutePath, const SerializationBinaryFileTables* fileTables, TimeStamp fileTimestamp);

    virtual AsyncFileBufferLoaderPtr createAsyncBufferLoader(uint64_t bufferID) const override final;

private:
    StringBuf m_absolutePath;

    const SerializationBinaryFileTables* m_fileTables = nullptr;
    TimeStamp m_fileTimestamp;
};
#endif

//--

END_INFERNO_NAMESPACE()
