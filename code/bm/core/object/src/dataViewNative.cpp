/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: view #]
***/

#include "build.h"
#include "dataViewNative.h"

#include "rttiDataView.h"
#include "rttiClassType.h"
#include "action.h"
#include "rttiDataHolder.h"
#include "object.h"
#include "rttiHandleType.h"

BEGIN_INFERNO_NAMESPACE()

//----

DataViewNative::DataViewNative(IObject* obj, bool readOnly)
    : m_object(AddRef(obj))
    , m_events(this)
    , m_readOnly(readOnly)
{
    if (m_object)
    {
        m_events.bind(m_object->eventKey(), EVENT_OBJECT_PROPERTY_CHANGED) = [this](StringBuf path)
        {
            dispatchPropertyChanged(path);
        };

        m_events.bind(m_object->eventKey(), EVENT_OBJECT_STRUCTURE_CHANGED) = [this]()
        {
            dispatchFullStructureChanged();
        };
    }
}

DataViewNative::~DataViewNative()
{
}

DataViewResult DataViewNative::describeDataView(StringView viewPath, DataViewInfo& outInfo) const
{
    if (m_object)
    {
        const auto result = m_object->describeDataView(viewPath, outInfo);
        if (m_readOnly && result.valid())
            outInfo.flags |= DataViewInfoFlagBit::ReadOnly;
        return result;
    }
        
    return DataViewResultCode::ErrorNullObject;
}

DataViewResult DataViewNative::readDataView(StringView viewPath, void* targetData, Type targetType) const
{
    if (m_object)
        return m_object->readDataView(viewPath, targetData, targetType);

    return DataViewResultCode::ErrorNullObject;
}

DataViewResult DataViewNative::readDefaultDataView(StringView viewPath, void* targetData, Type targetType) const
{
    if (m_object && m_object->cls())
    {
        if (m_object->cls()->isAbstract())
            return DataViewResultCode::ErrorIllegalAccess;

        if (const void* defaultObject = m_object->cls()->defaultObject())
            return m_object->cls()->readDataView(viewPath, defaultObject, targetData, targetType);
    }

    return DataViewResultCode::ErrorNullObject;
}

DataViewResult DataViewNative::writeDataView(StringView viewPath, const void* sourceData, Type sourceType) const
{
    // TODO: prevalidate ?

    if (m_readOnly)
        return DataViewResultCode::ErrorReadOnly;

    if (m_object)
        return m_object->writeDataView(viewPath, sourceData, sourceType);

    return DataViewResultCode::ErrorNullObject;
}

DataViewResult DataViewNative::resetToDefaultValue(StringView viewPath, void* targetData, Type targetType) const
{
    return writeDataView(viewPath, targetData, targetType);
}

//----

struct ActionWriteProperty : public IAction
{
public:
    ActionWriteProperty(const DataViewNative* view, StringView viewPath, DataHolder&& oldValue, DataHolder&& newValue, bool oldValueIdDefault=false, bool newValueIsDefault=false)
        : m_newValue(std::move(newValue))
        , m_oldValue(std::move(oldValue))
        , m_newValueIsDefaultValue(newValueIsDefault)
        , m_oldValueIsDefaultValue(oldValueIdDefault)
        , m_view(AddRef(view))
        , m_path(viewPath)
    {}

    virtual StringID id() const override
    {
        return "WriteProperty"_id;
    }

    StringBuf description() const override
    {
        return TempString("Change property '{}'", m_path);
    }

    virtual bool execute() override
    {
        if (m_newValueIsDefaultValue)
            return m_view->resetToDefaultValue(m_path, m_newValue.data(), m_newValue.type()).valid();
        else
            return m_view->writeDataViewSimple(m_path, m_newValue).valid();
    }

    virtual bool undo() override
    {
        if (m_oldValueIsDefaultValue)
            return m_view->resetToDefaultValue(m_path, m_oldValue.data(), m_oldValue.type()).valid();
        else
            return m_view->writeDataViewSimple(m_path, m_oldValue).valid();            
    }

    virtual bool tryMerge(const IAction& lastUndoAction)
    {
        /*if (lastUndoAction.id() == id())
        {
            const auto& lastWriteValue = static_cast<const ActionWriteProperty&>(lastUndoAction);
            if (lastWriteValue.m_path == m_path && lastWriteValue.m_view == m_view)
            {
                if (lastWriteValue.m_newValue == m_oldValue)
                {
                    m_oldValue = lastWriteValue.m_oldValue;
                    return true;
                }
            }
        }*/

        return false;
    }

private:
    DataHolder m_newValue;
    DataHolder m_oldValue;

    bool m_newValueIsDefaultValue = false;
    bool m_oldValueIsDefaultValue = false;

    DataViewNativePtr m_view;
    StringBuf m_path;
};

bool DataViewNative::checkIfCurrentlyADefaultValue(StringView viewPath) const
{
    return false;
}

#define RUN_SAFE(x) { auto ret = x; if (!ret.valid()) return ret; }

DataViewActionResult DataViewNative::actionValueWrite(StringView viewPath, const void* sourceData, Type sourceType) const
{
    // protect read only objects
    if (m_readOnly)
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // describe the data view to know is the value is not read only
    DataViewInfo info;
    info.requestFlags |= DataViewRequestFlagBit::CheckIfResetable;
    RUN_SAFE(describeDataView(viewPath, info));

    // oh well
    if (info.flags.test(DataViewInfoFlagBit::ReadOnly))
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // read the current value first - that's the value we will restore to in case of undo
    // NOTE: any failure here usually means that we can't write to this value either
    DataHolder currentValue(sourceType);
    RUN_SAFE(readDataViewSimple(viewPath, currentValue));

    // if the current value is the "default
    const auto currentValueIsDefaultValue = checkIfCurrentlyADefaultValue(viewPath);

    // create final write action
    DataHolder newValue(sourceType, sourceData);
    return DataViewActionResult(RefNew<ActionWriteProperty>(this, viewPath, std::move(currentValue), std::move(newValue), currentValueIsDefaultValue, false));
}

//--

DataViewActionResult DataViewNative::actionValueReset(StringView viewPath) const
{
    // protect read only objects
    if (m_readOnly)
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // describe the data view to know the type and if it even can be reset
    DataViewInfo info;
    info.requestFlags |= DataViewRequestFlagBit::CheckIfResetable;
    RUN_SAFE(describeDataView(viewPath, info));

    // oh well
    if (info.flags.test(DataViewInfoFlagBit::ReadOnly))
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // well, the property is not resettable, there's no point (also this is protection of sorts as some properties like "name" or "ID") should not be resetable because the "default value" is usually invalid (empty)
    if (!info.flags.test(DataViewInfoFlagBit::ResetableToBaseValue))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // read the default value from the CLASS, this will only work for simple 
    DataHolder defaultValue(info.dataType);
    RUN_SAFE(readDefaultDataView(viewPath, defaultValue.data(), defaultValue.type()));

    // read the current value first - that's the value we will restore to in case of undo
    // NOTE: any failure here usually means that we can't write to this value either
    DataHolder currentValue(info.dataType);
    RUN_SAFE(readDataViewSimple(viewPath, currentValue));

    // if the current value is the "default
    const auto currentValueIsDefaultValue = checkIfCurrentlyADefaultValue(viewPath);
    return DataViewActionResult(RefNew<ActionWriteProperty>(this, viewPath, std::move(currentValue), std::move(defaultValue), currentValueIsDefaultValue, true));
}

DataViewActionResult DataViewNative::actionArrayClear(StringView viewPath) const
{
    // protect read only objects
    if (m_readOnly)
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // describe the data view to know the type and if it even can be reset
    DataViewInfo info;
    RUN_SAFE(describeDataView(viewPath, info));

    // oh well
    if (info.flags.test(DataViewInfoFlagBit::ReadOnly))
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // we need an array
    if (!info.flags.test(DataViewInfoFlagBit::LikeArray))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // read current array content
    DataHolder currentValue(info.dataType);
    RUN_SAFE(readDataViewSimple(viewPath, currentValue));

    // write empty data (empty array)
    DataHolder newValue(info.dataType);
    return DataViewActionResult(RefNew<ActionWriteProperty>(this, viewPath, std::move(currentValue), std::move(newValue)));
}

DataViewActionResult DataViewNative::actionArrayInsertElement(StringView viewPath, uint32_t index) const
{
    // TODO
    return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);
}

DataViewActionResult DataViewNative::actionArrayRemoveElement(StringView viewPath, uint32_t index) const
{
    // TODO
    return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);
}

DataViewActionResult DataViewNative::actionArrayNewElement(StringView viewPath) const
{
    // TODO
    return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);
}
    
//---

DataViewActionResult DataViewNative::actionObjectClear(StringView viewPath) const
{
    // describe the data view to know the type and if it even can be reset
    DataViewInfo info;
    RUN_SAFE(describeDataView(viewPath, info));

    // oh well
    if (info.flags.test(DataViewInfoFlagBit::ReadOnly))
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // we need an object type
    if (!info.flags.test(DataViewInfoFlagBit::Object))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // we MUST be inlined
    if (!info.flags.test(DataViewInfoFlagBit::Inlined))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // type must be a handle
    if (!info.dataType || info.dataType->metaType() != MetaType::StrongHandle)
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // read current array content
    DataHolder currentValue(info.dataType);
    RUN_SAFE(readDataViewSimple(viewPath, currentValue));

    // read the current object pointer
    ObjectPtr objectPtr;
    static const auto* handleType = static_cast<const IHandleType*>(info.dataType.ptr());
    handleType->readPointedObject(currentValue.data(), objectPtr);

    // if object exists it will be unparented
    if (objectPtr)
    {
        DEBUG_CHECK_EX(objectPtr->parent() == m_object, "Inlined object not parented to parent object")
    }

    // write empty data (empty pointer)
    DataHolder newValue(info.dataType);
    return DataViewActionResult(RefNew<ActionWriteProperty>(this, viewPath, std::move(currentValue), std::move(newValue)));
}

DataViewActionResult DataViewNative::actionObjectNew(StringView viewPath, ClassType objectClass) const
{
    // describe the data view to know the type and if it even can be reset
    DataViewInfo info;
    RUN_SAFE(describeDataView(viewPath, info));

    // oh well
    if (info.flags.test(DataViewInfoFlagBit::ReadOnly))
        return DataViewActionResult(DataViewResultCode::ErrorReadOnly);

    // we need an object type
    if (!info.flags.test(DataViewInfoFlagBit::Object))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // we MUST be inlined
    if (!info.flags.test(DataViewInfoFlagBit::Inlined))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // can't create object of abstract class
    if (!objectClass || objectClass->isAbstract())
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // type must be a handle
    if (!info.dataType || info.dataType->metaType() != MetaType::StrongHandle)
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // read the current object pointer and check if class is compatible
    static const auto* handleType = static_cast<const IHandleType*>(info.dataType.ptr());
    if (!objectClass->is(handleType->pointedClass()))
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // read current array content
    DataHolder currentValue(info.dataType);
    RUN_SAFE(readDataViewSimple(viewPath, currentValue));

    // create new object of given class
    ObjectPtr objectPtr = objectClass.create<IObject>();
    if (!objectPtr)
        return DataViewActionResult(DataViewResultCode::ErrorIllegalOperation);

    // parent the object! this is crucial and believe me, it's crazy to find a good place for this call
    objectPtr->parent(m_object);

    // store as a new pointer
    DataHolder newValue(info.dataType);
    handleType->writePointedObject(newValue.data(), objectPtr);

    // switch to new pointer
    return DataViewActionResult(RefNew<ActionWriteProperty>(this, viewPath, std::move(currentValue), std::move(newValue)));
}

//----

END_INFERNO_NAMESPACE()

