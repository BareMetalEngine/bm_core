/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/
#include "build.h"
#include "serializationBufferFactory.h"

BEGIN_INFERNO_NAMESPACE()

//--

class ObjectLoadingErrorCollector;

class ISerializationBufferFactory;

class SerializationBinaryFileTables;
struct SerializationResolvedReferences;

//--

// loader of binary serialized objects
class BM_CORE_OBJECT_API ObjectBinaryLoader : public MainPoolData<NoCopy>
{
public:
    enum class FileLoadMode
    {
        Full, // load full file
        DepsOnly, // load only basic data
        NoBuffers, // load objects without buffers
    };

    static bool LoadObjects(const ObjectLoadingContext& context, BufferView data, ObjectPtr& outRoot);
    static bool LoadFileContent(IFileView* file, Buffer& outData, FileLoadMode loadMode);
    static bool LoadDependencies(const ObjectLoadingContext& context, BufferView data, HashSet<ResourceID>& outDependencies);

    static bool LocateBufferPlacement(BufferView data, uint64_t crc, ISerializationBufferFactory::BufferInfo& outInfo);

private:
    static void ResolveStringIDs(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences);
    static void ResolveTypes(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences);
    static void ResolveProperties(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences);
    static void ResolveImports(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences);
    static void ResolveExports(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences, ObjectPtr& outResult);
    static void ResolveBuffers(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences, BufferView data);
};

//--

END_INFERNO_NAMESPACE()
