/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

// type of embedded file
enum class EmbeddedFileType : uint8_t
{
    Generic, // generic file embedded as-is
    Shader, // compiled platform specific shader file
    Image, // loaded image
    Mesh, // simple 3D mesh data
    Styles, // compiled style sheets

    MAX,
};

// dependency of embedded file (source file)
struct EmbeddedFileSource
{
    const char* sourcePath = nullptr; // source path file was embedded from (could be used for reloading)
    TimeStamp sourceTimestamp; // file timestamp as it was embedded
    EmbeddedFileSource* nextSource = nullptr;
};

// general embedded file
struct EmbeddedFile
{
    EmbeddedFileType type = EmbeddedFileType::Generic;
    const char* name = nullptr; // local embedded file name (can be whatever string)
    const EmbeddedFileSource* sources = nullptr;
};

// generic embedded file
struct EmbeddedFile_Generic : public EmbeddedFile
{
    uint32_t size = 0;
    const uint8_t* data = nullptr;
};

// compiled style sheets
struct EmbeddedFile_Styles : public EmbeddedFile
{
    uint32_t size = 0;
    const uint8_t* data = nullptr;
};

// embedded (unpacked) image
struct EmbeddedFile_Image : public EmbeddedFile
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    uint8_t channels = 0;
    uint8_t format = 0;
    const uint8_t* pixels = nullptr;
};

// embedded shader platform payload
struct EmbeddedFile_ShaderPlatformPayload
{
    const char* name = nullptr; // platform name "DX12, DX11, VK10, VL11, GLSL4, EGLSL etc"
    char platform = -1;
    uint32_t size = 0;
    const uint8_t* data = nullptr; // serialized shader data
    EmbeddedFile_ShaderPlatformPayload* next = nullptr; // next platform payload
};

// payload for embedded shader
struct EmbeddedFile_Shader : public EmbeddedFile
{
    const char* defines = nullptr; // semicolon separated list of defines used to compile the shader
    const char* includePaths = nullptr; // semicolon separate list of include paths used to compile the shader
    const EmbeddedFile_ShaderPlatformPayload* platformPayloads = nullptr;
};

// embedded mesh vertex
struct EmbeddedVertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    uint32_t color;
};

// embedded mesh
struct EmbeddedFile_Mesh : public EmbeddedFile
{
    const EmbeddedVertex* vertices = nullptr;
    const uint32_t* indices = nullptr;
    uint32_t numVertices = 0;
    uint32_t numIndices = 0;
};

//--

// registry of all embedded files
class BM_CORE_FILE_API IEmbeddedFilesRegistry : public MainPoolData<NoCopy>
{
public:
    // find embedded file by name
    virtual const EmbeddedFile* findFile(const StringView name, EmbeddedFileType type) const = 0;

    // create file reader for a embedded file, works only on generic files that were embedded as-is
    virtual FileReaderPtr openFile(const StringView name) const = 0;

    // iterate over all embedded files
    virtual void iterateFiles(EmbeddedFileType type, const std::function<void(const EmbeddedFile*)>& func) const = 0;

    // register embedded file, usually done during module loading
    virtual void registerFile(const EmbeddedFile* file) = 0;

    // unregister embedded file, usually done during module unloading
    virtual void unregisterFile(const EmbeddedFile* file) = 0;

protected:
    IEmbeddedFilesRegistry();
    virtual ~IEmbeddedFilesRegistry();
};

// registry of embedded files
extern BM_CORE_FILE_API IEmbeddedFilesRegistry& EmbeddedFiles();

//--

END_INFERNO_NAMESPACE()
