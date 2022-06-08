/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// low-level serialization error reporter
class BM_CORE_OBJECT_API ISerializationErrorReporter : public MainPoolData<NoCopy>
{
public:
    virtual ~ISerializationErrorReporter();

    /// type is missing from file, type may be a class or sth else
    virtual void reportTypeMissing(const StringBuf& serializationContextName, StringID missingTypeName) = 0;

    /// property is missing from class
    virtual void reportPropertyMissing(const StringBuf& serializationContextName, StringID classType, StringID missingPropertyName, StringID propertyType, bool handled) = 0;

	/// property changed it's type to one that does not exist any more
	virtual void reportPropertyTypeMissing(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID previousType, bool handled) = 0;

    /// property changed it's type 
    virtual void reportPropertyTypeChanged(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID previousType, StringID currentType, bool handled) = 0;

    /// property failed to deserialize property
    virtual void reportPropertyReadFailed(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID propertyType) = 0;

    /// enum is missing value
    virtual void reportPropertyEnumMissing(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID enumType, StringID enumOptionName) = 0;

	/// bitflags are missing value
	virtual void reportPropertyBitflagMissing(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID enumType, StringID enumOptionName) = 0;

    ///--

    // get det default error reporter - prints it to log
    static ISerializationErrorReporter& DefaultErrorReporter();

	///---
};

//---

END_INFERNO_NAMESPACE()
