/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

// Glue code
#include "bm_core_object_glue.inl"

//--

BEGIN_INFERNO_NAMESPACE()

//--

/// maximum number of template properties in any given class
static const uint32_t MAX_TEMPLATE_PROPERTIES = 128;

//--

/// Event in the global system
/// TODO: make something more useful thank StringID
#define DECLARE_GLOBAL_EVENT(x, ...) \
    inline static const ::bm::StringID x = #x##_id;

DECLARE_GLOBAL_EVENT(EVENT_OBJECT_PROPERTY_CHANGED);
DECLARE_GLOBAL_EVENT(EVENT_OBJECT_STRUCTURE_CHANGED);

//--

/// stubs
struct IStub;
class IStubWriter;
class IStubReader;

typedef uint16_t StubTypeValue;

//---

/// object pointer types
class IObject;
typedef RefPtr<IObject> ObjectPtr;
typedef RefWeakPtr<IObject> ObjectWeakPtr;

/// offset from the object start to the script data table
static const uint32_t OBJECT_OFFSET_TO_SCRIPT_DATA = 32;

/// ID of resource
class ResourceID;

/// resource - file based object
class IResource;
typedef RefPtr<IResource> ResourcePtr;
typedef RefWeakPtr<IResource> ResourceWeakPtr;

/// resource proxy
class ResourcePromise;
typedef RefPtr<ResourcePromise> ResourcePromisePtr;

class IResourcePromiseFactory;

/// observer of object's state
class IObjectObserver;

/// abstract data view
class IDataView;
typedef RefPtr<IDataView> DataViewPtr;

/// data view of an object
class DataViewNative;
typedef RefPtr<DataViewNative> DataViewNativePtr;

// actions
class IAction;
typedef RefPtr<IAction> ActionPtr;
class ActionHistory;
typedef RefPtr<ActionHistory> ActionHistoryPtr;

/// direct object template
class IObjectDirectTemplate;
typedef RefPtr<IObjectDirectTemplate> ObjectDirectTemplatePtr;
typedef RefWeakPtr<IObjectDirectTemplate> ObjectDirectTemplateWeakPtr;

// global events
class IGlobalEventListener;
typedef RefPtr<IGlobalEventListener> GlobalEventListenerPtr;

// selection
class Selectable;
struct EncodedSelectable;

// async buffer
class IAsyncFileBufferLoader;
typedef RefPtr<IAsyncFileBufferLoader> AsyncFileBufferLoaderPtr;

//--

class ICommand;
typedef RefPtr<ICommand> CommandPtr;

//--

/// Meta type (type of type)
enum class MetaType
{
    Void,           // empty type (used to represent invalid type of no value)
    Simple,         // simple type (indivisible into smaller pieces)
    Enum,           // EnumType
    Bitfield,       // BitfieldType
    Class,          // ClassType
    Array,          // IArrayType - array type, may have different implementations
    StrongHandle,   // StringHandleType - handle to object that keeps it alive
    WeakHandle,     // WeakHandleType - handle to object that does not keep it alive
    ResourceRef,    // IReferenceType - resource reference
    ClassRef,       // SpecificClassType - reference to class
};

// forward declarations of RTTI types
class IType;
class AccessPath;
class IClassType;
class EnumType;
class BitfieldType;
class IArrayType;
class IHandleType;
class TypeSystem;
class IFunction;
class NativeFunction;
class IScriptFunction;
class Property;
class CustomType;
class IClassType;
class NativeClass;
class IType;

class ClassType;

template< typename T >
class SpecificClassType;

//--

class SerializationReader;
class SerializationWriter;

struct SerializationPropertyInfo;
struct SerializationResourceInfo;
struct SerializationResourceKey;

class ISerializationErrorReporter;

enum class SerializationFormat : uint8_t
{
	RawBinary,
	XML,
	BinaryXML,
	Json,
};

//--

class ITextSerializationWriter;
class ITextSerializationReader;

//--

typedef uint32_t ResourceUniqueID;
typedef uint32_t ResourceRuntimeVersion;

class ResourceID;
class ResourceProxy;

class IResource;
typedef RefPtr<IResource> ResourcePtr;
typedef RefWeakPtr<IResource> ResourceWeakPtr;

class BaseReference;
class BaseAsyncReference;

template< typename T >
class ResourceRef;

template< typename T >
class ResourceAsyncRef;

typedef SpecificClassType<IResource> ResourceClass;

class Variant;
class VariantTable;

class TypedMemory;

//--

struct ObjectSavingContext;
struct ObjectLoadingContext;

struct TypeSerializationContext;

class ISerializationBufferFactory;

//--

/// styling params for text printing
/// NOTE: shard between all writers
enum class DataTextFormatterFlagBit : uint8_t
{
	Relaxed, // do not print the "," between elements, try to drop as many quotes as possible
	UseNewLines, // add line breaks
	UseIndentation, // add indentation
};

// formatting flags
typedef BitFlags<DataTextFormatterFlagBit> DataTextFormatterFlags;

// abstract class printing text representation of data, used EVERYWHERE, especially to make JSON/RJSON data
class IDataTextFormatter;

//--

struct DataViewInfo;
struct DataViewMemberInfo;
struct DataViewOptionInfo;

enum class DataViewResultCode : uint8_t
{
    OK,
    ErrorUnknownProperty, // property (or part of path) is unknown (ie. property does not exist)
    ErrorIllegalAccess, // you can't access that property like that (ie. hidden property)
    ErrorIllegalOperation, // operation is not permitted - ie. resizing static array
    ErrorTypeConversion, // property exists and is accessible but was not readable as the type that was requested
    ErrorManyValues, // property has many different values and a single value cannot be established
    ErrorReadOnly, // property is marked as read only, no writes are possible
    ErrorNullObject, // trying to access null object
    ErrorIndexOutOfRange, // trying to access array's index that is out of range
    ErrorInvalidValid, // value to write is invalid
    ErrorIncompatibleMultiView, // field with the same name in different views in a multi view is something completly different 
};

struct BM_CORE_OBJECT_API DataViewResult
{
    DataViewResultCode code = DataViewResultCode::ErrorIllegalOperation;
    StringBuf additionalInfo;


    INLINE DataViewResult(DataViewResultCode code_) : code(code_) {};

    INLINE bool valid() const { return code == DataViewResultCode::OK; }

    void print(IFormatStream& f) const;

    INLINE DataViewResult() = default;
    INLINE DataViewResult(const DataViewResult & other) = default;
    INLINE DataViewResult(DataViewResult && other) = default;
    INLINE ~DataViewResult() = default;
    INLINE DataViewResult& operator=(const DataViewResult & other) = default;
    INLINE DataViewResult& operator=(DataViewResult && other) = default;
};

//---

class BM_CORE_OBJECT_API IHackyFontGlyphCacheIntegration : public MainPoolData<NoCopy>
{
public:
    virtual ~IHackyFontGlyphCacheIntegration();

    static void PullNewData(Array<uint8_t>& outData, uint32_t& outOffset);

protected:
    virtual void pullNewData(Array<uint8_t>& outData, uint32_t& outOffset) = 0;
    static void RunWithPointer(const std::function<void(IHackyFontGlyphCacheIntegration*&)>& func);
};

//---

END_INFERNO_NAMESPACE()

//---

#include "rttiTypeSystem.h"
#include "rttiTypeRef.h"
#include "rttiClassRefType.h"
#include "rttiClassRef.h"
#include "rttiTypedMemory.h"

#include "object.h"

#include "globalEventDispatch.h"
#include "globalEventTable.h"

#include "reflectionMacros.h"

#include "serializationVersions.h"

#include "configProperty.h"

//---