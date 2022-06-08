/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "globalEventKey.h"
#include "rttiDataHolder.h"
#include "rttiClassRef.h"
#include "rttiClassType.h"
#include "rttiProperty.h"
#include "reflectionMacros.h"
#include "resourceID.h"

#include "bm/core/containers/include/stringID.h"
#include "bm/core/containers/include/hashSet.h"

// Helper macros for old stuff
#define NO_DEFAULT_CONSTRUCTOR( _class )    \
    protected: _class() {};

// Event function
#define OBJECT_EVENT_FUNC StringID eventID, const IObject* object, StringView eventPath, const DataHolder& eventData

BEGIN_INFERNO_NAMESPACE()

//---

namespace backend
{
    class IDataBlobRespository;
    typedef bm::GUID DataBlobID;
} // packend

//---

class MonoEnvironment;
class ScriptedClassType;
struct ObjectSavingExtractedData;
struct ObjectCloneContext;

//---

/// helper interface to list object's template properties
class BM_CORE_OBJECT_API ITemplatePropertyBuilder : public MainPoolData<NoCopy>
{
public:
    virtual ~ITemplatePropertyBuilder();

    virtual void prop(StringID category, StringID name, Type type, const void* defaultValue, const PropertyEditorData& editorData) = 0;

    template< typename T >
    inline void prop(StringID category, StringID name, const T& defaultValue = T(), const PropertyEditorData& editorData = PropertyEditorData())
    {
        prop(category, name, GetTypeObject<T>(), &defaultValue, editorData);
    }
};

/// helper interface to provide values for object template properties
class BM_CORE_OBJECT_API ITemplatePropertyValueContainer : public MainPoolData<NoCopy>
{
public:
    virtual ~ITemplatePropertyValueContainer();

    virtual ClassType compileClass() const = 0;

    virtual bool compileValue(StringID name, Type expectedType, void* ptr) const = 0;

    template< typename T >
    inline bool compileValue(StringID name, T& ptr) const
    {
        return compileValue(name, GetTypeObject<T>(), &ptr);
    }

    template< typename T >
    inline T compileValueOrDefault(StringID name, T defaultValue = T()) const
    {
        T ret;
        if (compileValue(name, GetTypeObject<T>(), &ret))
            return ret;
        return defaultValue;
    }
};

//--

// object ID Type
typedef uint32_t ObjectID;

//--

/// General object loading context
struct BM_CORE_OBJECT_API ObjectLoadingContext : public NoCopy
{
    IPoolUnmanaged* pool = nullptr;

	ISerializationBufferFactory* bufferFactory = nullptr;

    StringBuf contextPath;
    bool preloadBuffer = false;

    ClassType expectedRootClass;

    HashSet<ClassType> selectiveLoadingClasses;
    Array<ObjectPtr>* collectedSelectiveObjects = nullptr;

	Array<ResourcePromisePtr>* resourcePromises = nullptr;

    Array<ObjectPtr>* collectedRootObjects = nullptr;
	ISerializationErrorReporter* collectedErrors = nullptr;

	// [optimization] when loading from a buffer that we can persist all embedded buffers can be effectively just views into the parent
	Buffer dataOwnerToInheritOtherUncompressedBuffers;

    //--

    ObjectLoadingContext();
};

//--

// extracted buffer data
struct ObjectExtractedBuffer
{
	uint64_t uncompressedCRC = 0;
	uint64_t uncompressedSize = 0;
	CompressionType compressionType;
	Buffer compressedData; // compressed buffer data (or not if it did not compress well)
};

/// Saving context
struct BM_CORE_OBJECT_API ObjectSavingContext : public NoCopy
{
    IPoolUnmanaged* pool = nullptr;
    IPoolPaged* pagedPool = nullptr;

    bool decompressAllBuffer = false;
    bool inlineAllBuffer = false;
    bool inlineAllResources = false;

    PrintFlags textPrintFlags;

    HashSet<SerializationResourceKey>* extractedResources = nullptr;
    HashMap<uint64_t, ObjectExtractedBuffer>* extractedBuffers = nullptr;

    StringView rootNodeName;

    ObjectSavingContext();
};

//--

/// Base class of manageable objects
/// NOTE: objects are NOT copyable, period
class BM_CORE_OBJECT_API IObject : public IReferencable
{
	DECLARE_POOL_WRAPPERS_NEW_PATCH(IObject)

public:
    IObject();
    virtual ~IObject();

    //---

    // Get unique object ID, this number will never repeat (pointer might though...)
    INLINE ObjectID id() const { return m_id; }

    // Get event ID we can use to listen for events on this object
    INLINE GlobalEventKey eventKey() const { return m_eventKey; }

    // Get parent object, does not have to be defined
    INLINE IObject* parent() const { return m_parent.unsafe(); }

    // Get parent object, does not have to be defined
    INLINE ObjectPtr parentRef() const { return m_parent.lock(); }

    // Buffer with scripted data for object
    INLINE const void* scriptData() const { return m_scriptData; }

    // Buffer with scripted data for object
    INLINE void* scriptData() { return m_scriptData; }

    // Check if object is of given class
    template< class T > INLINE bool is() const;

    // Check if object is of given class
    bool is(ClassType objectClass) const;

    // Check if object is of given class
    template< typename T >
    INLINE bool is(SpecificClassType<T> objectClass) const;

    //---

    // Get static object class, overloaded in child classes
    static SpecificClassType<IObject> GetStaticClass();

    // Get object native class, dynamic but C++ class always
    virtual ClassType nativeCls() const { return GetStaticClass(); }

    // Get object actual runtime class
    virtual ClassType cls() const { return m_class; }

    // Print object description, usually just class name but for named objects or objects with captions we can print their name as well
    virtual void print(IFormatStream& f) const override;

    //---

    // Called after object and all child objects were loaded and before the object is returned from job
    virtual void onPostLoad();

    // Load object from binary stream
    virtual void onReadBinary(SerializationReader& reader);

    // Save object to binary stream
    virtual void onWriteBinary(SerializationWriter& writer) const;

	// We are reading object from binary stream and previously serialized property is no longer in the object and the type does not exist
	virtual bool onPropertyMissingRaw(const SerializationPropertyInfo& info, SerializationReader& reader);

    // We are reading object from text/binary stream and previously serialized property is no longer in the object
    virtual bool onPropertyMissing(StringID propertyName, TypedMemory data);

	// We are reading object from binary stream and previously serialized property has different type that does not exist now
	virtual bool onPropertyTypeChangedRaw(const SerializationPropertyInfo& info, TypedMemory propertyData, SerializationReader& reader);

    // We are reading object from text/binary stream and previously serialized property has different type than now AND we could not handle the conversion automatically
    virtual bool onPropertyTypeChanged(StringID propertyName, TypedMemory propertyData, TypedMemory loadedData);

    //---

    // Mark this object as modified, this propagates up the object hierarchy eventually (hopefully) reaching the parent resource
    // This is the main functionality used to track editor side changes to objects
    virtual void markModified();

    //---

    /// Get metadata for view - describe what we will find here: flags, list of members, size of array, etc
    virtual DataViewResult describeDataView(StringView viewPath, DataViewInfo& outInfo) const;

    /// Read data from memory
    virtual DataViewResult readDataView(StringView viewPath, void* targetData, Type targetType) const;

    /// Write data to memory
    virtual DataViewResult writeDataView(StringView viewPath, const void* sourceData, Type sourceType);

    //---

	/// Write object into XML node (note: node must exist before, the "class" attribute is not written, it's responsibility of the parent)
	virtual void writeText(ITextSerializationWriter& writer) const;

	/// Load object from XML node, object must be created beforehand
	virtual void readText(ITextSerializationReader& reader);
    
    //---

    // Notification about property change in progress, can be denied if the value is not ok
    // Object should return true to accept the change or false to reject it
    virtual bool onPropertyChanging(StringView path, const void* newData, Type newDataType) const;

    // Notification about property change that occurred
    virtual void onPropertyChanged(StringView path);

    // Filter visibility of given property in the data view of this object
    // Can be used to conditionally mask/unmask properties based on some part of object configuration
    // NOTE: the property that is used for masking must call postEvent("OnFullObjectRefresh") on the object to be able to refresh the UI properly.
    // NOTE: this function is called only when the property is initially requested (when user expands node in the property grid)
    virtual bool onPropertyFilter(StringID propertyName) const;

    //---

    // Check if property should be saved during serialization, usually properties that are the same as in base are filtered out
    virtual bool onPropertyShouldSave(const Property* prop) const;

    // Notifies that property value was found in file - we can skip loading it for some reason
    virtual bool onPropertyShouldLoad(const SerializationPropertyInfo& prop);

    //---

    // Patch resource pointers - called during reloading, can be used to use custom implementation, should return true if anything was patched
    // This function is called GLOBALLY ON ALL OBJECTSs when a in-editor resource reload occurs, by default it automatically patches up properties referencing that object
    // NOTE: this function should return true if it did some patching and it requires some post-reload work (ie. after all other objects were patched)
    // NOTE: the automatic patching will call "onPropertyChanged" on each changed property
    virtual bool onResourceReloading(IResource* currentResource, IResource* newResource);

    // Called after all objects were patched after resource reload
    virtual void onResourceReloadFinished(IResource* currentResource, IResource* newResource);

    //---

    // set parent object for this object
    // parent objects are used to determine what hierarchy of objects should be saved together
    void parent(const IObject* parentObject);

    // check if this object has a given parent
    bool hasParent(const IObject* parentObject) const;

    // find parent of specific class
    IObject* findParent(ClassType parentObjectClass) const;

    // find parent of specific class
    template< typename T >
    INLINE T* findParent() const { return static_cast<T*>(findParent(T::GetStaticClass())); }

    //---

    // initialize class (since we have no automatic reflection in this project)
    static void RegisterType(TypeSystem& typeSystem);

    //---

    /*// run some code on main thread in context of this object (assuming it was not deleted before)
    template< typename T >
    void runSync(const std::function<void(T& obj)>& func)
    {
        auto weakSelf = RefWeakPtr<T>(rtti_cast<T>(this));
        FiberBuilder().name("ObjectSyncJob").sync() << [weakSelf, func]()
        {
            if (auto obj = weakSelf.lock())
                func(*obj);
        };
    }

    // run some code on main thread in context of this object (assuming it was not deleted before)
    void runSync(const std::function<void()>& func)
    {
        auto weakSelf = RefWeakPtr<IObject>(this);
        FiberBuilder().name("ObjectSyncJob").sync() << [weakSelf, func]()
        {
            if (auto obj = weakSelf.lock())
                func();
        };
    }*/

    //---

    // post event from this object to all interested object's observers
    // NOTE: this is intended for editor or non-time critical functionality as most of the events will be routed through main thread
    // NOTE: if calling function is running on main thread we will execute callbacks immediately, if it's not the even will be posted to be executed later on main thread
    void postEvent(StringID eventID, const void* data = nullptr, Type dataType = Type());

    // post event from this object to all interested object's observers
    template< typename T >
    INLINE void postEvent(StringID eventID, const T & data)
    {
        postEvent(eventID, &data, GetTypeObject<T>());
    }

    //--

    // allocate unique object ID
    static ObjectID AllocUniqueObjectID();

    // get object by ID, global, slow, used mostly by global event system
    static ObjectPtr FindUniqueObjectById(ObjectID id);

    //--

    // clone this object (and all linked child objects)
    // NOTE: any references to objects outside the hierarchy will be nullified, beware!
    // NOTE: any loaded resource references will still be loaded any unloaded ones will be loaded (slow...)
    ObjectPtr clone() const;

    // clone this object within given object translation context
    virtual ObjectPtr cloneSelf(ObjectCloneContext& ctx) const;

    // copy object properties
    virtual void cloneProperties(ObjectCloneContext& ctx, IObject* target) const;

    //--
    
    // create data view for editing this object, especially in data inspector + data view is usually needed for undo/redo
    virtual DataViewPtr createDataView(bool forceReadOnly=false) const;

    //--

    // list all template properties for this object
    // NOTE: slow function, results should be cached
    virtual void queryTemplateProperties(ITemplatePropertyBuilder& outTemplateProperties) const;

    // initialize from template properties
    virtual bool initializeFromTemplateProperties(const ITemplatePropertyValueContainer& templateProperties);

    //--

    // hacks to fight project dependencies...
    static void RegisterScriptObjetctDispose(const std::function<void(IObject*)>& func);
    static void RegisterScriptObjetctResurrect(const std::function<void(IObject*)>& func);

    //--

    /// copy value from one property to other property, handles type conversions, inlined objects etc
    static bool CopyPropertyValue(const IObject* srcObject, const Property* srcProperty, IObject* targetObject, const Property* targetProperty);

    //--
    
	// serialize object to text output (note: for binary formats this will output BASE64)
	static bool SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, IFormatStream& f);

	// serialize object and save it to file, works best for large data
	static bool SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, IFileWriter* writer);

    // serialize object and save it to a data buffer in memory, best for small objects
	static bool SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, Buffer& outBufer);

    // serialize object and save it directly to a file in a file system, best for very large data
	static bool SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, StringView absoluteFilePath, IFileSystem& fs = FileSystem());

    //--

	// load object from buffer in memory, may or may not load file references
	static ObjectPtr LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, StringView txt);

	// load object from an binary file 
    static ObjectPtr LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, BufferView data);

	// load object from an binary file 
    static ObjectPtr LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, IFileReader* file);

	// load object from an binary file 
	static ObjectPtr LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, StringView absoluteFilePath, IFileSystem& fs = FileSystem());

    //--

    // locate buffer placement in serialized file
    static bool LocateBufferPlacement(BufferView data, uint64_t crc, uint64_t& outBufferOffset, uint32_t& outBufferCompressedSize, uint32_t& outBufferUncompressedSize, CompressionType& outCompressionType);

    //--

    // load object from data in the memory
    template< typename T >
    static INLINE RefPtr<T> LoadObject(SerializationFormat format, BufferView data)
    {
        ObjectLoadingContext ctx;
        ctx.expectedRootClass = T::GetStaticClass();
        return rtti_cast<T>(LoadObject(format, ctx, data));
    }

	// load object from standalone file in a file system
	template< typename T >
	static INLINE RefPtr<T> LoadObject(SerializationFormat format, StringView absoluteFilePath, IFileSystem& fs)
	{
		ObjectLoadingContext ctx;
		ctx.expectedRootClass = T::GetStaticClass();
		return rtti_cast<T>(LoadObject(format, ctx, absoluteFilePath, fs));
	}

	// load object from a file
	template< typename T >
	static INLINE RefPtr<T> LoadObject(SerializationFormat format, IFileReader* file)
	{
		ObjectLoadingContext ctx;
		ctx.expectedRootClass = T::GetStaticClass();
		return rtti_cast<T>(LoadObject(format, ctx, file));
	}

    //--

    // clone object
    template< typename T >
    static INLINE RefPtr<T> CloneObject(const T* object)
    {
        return rtti_cast<T>(object ? object->clone() : nullptr);
    }

    // clone object
    template< typename T >
    static INLINE RefPtr<T> CloneObject(const RefPtr<T>& object)
    {
        return rtti_cast<T>(object ? object->clone() : nullptr);
    }

    //--

    // serialize to buffer
    Buffer toBuffer(SerializationFormat format = SerializationFormat::RawBinary, IPoolUnmanaged& pool = MainPool()) const;

    // serialize to string
    StringBuf toString(SerializationFormat format = SerializationFormat::Json) const;

    //--

protected:
    INLINE IObject(const IObject&) {};
    INLINE IObject& operator=(const IObject&) { return *this;  };

    //--

    // object's runtime class, can be different that native class if object is scripted
    // NOTE: the scripted class will always extend the object's native class
    const IClassType* m_class; // NOTE: not initialized in constructor because it's written from outside

    // object's script data block
    void* m_scriptData = nullptr;

    //--

    // objects form a hierarchy, mostly for the purpose of saving in resources, this is our pointer to parent object
    // NOTE: it's considered dick move to delete parent object without deleting the children
    ObjectWeakPtr m_parent;

    // unique (runtime) ID for this object
    ObjectID m_id = 0;

    // event key (TODO: merge somehow with ObjectID ?)
    GlobalEventKey m_eventKey;

    //--

    // patch class pointer in an object
    static void* PatchClassPointer(void* ptr, const IClassType* cls);

    //--

    friend class ScriptedClassType;
    friend class backend::IDataBlobRespository;

    virtual bool HACK_bindToDataBlob(backend::IDataBlobRespository* repo, const backend::DataBlobID& id) { return false; }
};

//--

END_INFERNO_NAMESPACE()


#include "object.inl"

