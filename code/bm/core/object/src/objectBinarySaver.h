/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/containers/include/queue.h"
#include "bm/core/containers/include/hashSet.h"
#include "serializationWriter.h"
#include "serializationStream.h"

BEGIN_INFERNO_NAMESPACE()

//--

struct SerializationMappedReferences;
class SerializationBinaryFileTables;
class SerializationBinaryFileTablesBuilder;

struct SerializedObject;
struct SerializationResourceKey;
class SerializedObjectCollection;

//--

// resource saver for native format
class BM_CORE_OBJECT_API ObjectBinarySaver : public MainPoolData<NoCopy>
{
public:
    static bool SaveObjects(IFileWriter* file, const ObjectSavingContext& context, ObjectPtr object);
    static void ExtractUsedResources(const IObject* object, HashMap<ResourceID, uint32_t>& outResourceCounts);

private:
    static bool CollectObjects(LocalAllocator& mem, const ObjectSavingContext& context, ObjectPtr object, SerializedObjectCollection& outCollection, SerializationWriterReferences& outReferences);
    static void BuildFileTables(const Array<SerializedObject*>& objects, const SerializationWriterReferences& collectedReferences, SerializationBinaryFileTablesBuilder& outTables, SerializationMappedReferences& outMappedReferences);
    static void WriteObjects(const ObjectSavingContext& context, const SerializedObjectCollection& objects, const SerializationMappedReferences& mappedReferences, SerializationBinaryFileTablesBuilder& tables, uint64_t baseOffset, IFileWriter* file);
    static uint32_t HeaderFlags(const ObjectSavingContext& context);
};

//--

END_INFERNO_NAMESPACE()
