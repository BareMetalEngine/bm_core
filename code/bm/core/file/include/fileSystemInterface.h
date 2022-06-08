/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// abstract file system interface used for reading with dependency tracking
class BM_CORE_FILE_API IFileSystemInterface : public MainPoolData<NoCopy>
{
public:
    virtual ~IFileSystemInterface();

    //--

    /// get file information - mostly timestamp and size, can be used to check if file exists
    virtual bool queryFileInfo(StringView path, TimeStamp* outTimestamp = nullptr, uint64_t* outSize = nullptr) const = 0;

    /// get absolute path for given virtual path, may fail for assets from packed file systems, usually used for printing stuff
    virtual bool queryFileAbsolutePath(StringView path, StringBuf& outAbsolutePath) const = 0;

    ///--

    /// helper: find a file relative to the folder of the context file
    /// For example assume you are importing "rendering/assets/meshes/box.fbx"
    /// Your texture path is "D:\Work\Assets\Box\Textures\wood.jpg"
    /// This function if check for existence of following files and pick the first that exists:
    ///  "rendering/assets/meshes/wood.jpg"
    ///  "rendering/assets/meshes/Textures/wood.jpg"
    ///  "rendering/assets/meshes/Box/Textures/wood.jpg"
    ///  "rendering/assets/wood.jpg"
    ///  "rendering/assets/Textures/wood.jpg"
    ///  "rendering/assets/Box/Textures/wood.jpg"
    ///  "rendering/wood.jpg"
    ///  "rendering/Textures/wood.jpg"
    ///  "rendering/Box/Textures/wood.jpg"
    ///  etc, up to the maxScanDepth
    virtual bool findRelativePath(StringView referencePath, StringView searchPath, StringBuf& outFoundAsssetPath, uint32_t maxScanDepth = 2) const;

    ///--

    /// load content of asset file to memory
    virtual Buffer loadContentToBuffer(StringView path, bool makeDependency = true) const = 0;

    /// load content of asset file to a string (main difference is that it does UTF conversion and appends zero at the end of the buffer)
    virtual StringBuf loadContentToString(StringView path, bool makeDependency = true) const;

    //--

    /// get the interface to physical file system
    static IFileSystemInterface& GetPhysicalFileSystemInterface();

    //--
};

//--

END_INFERNO_NAMESPACE()
