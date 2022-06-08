/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "serializationErrorReporter.h"

BEGIN_INFERNO_NAMESPACE()

//--

ISerializationErrorReporter::~ISerializationErrorReporter()
{}

///--

namespace prv
{
	class DefaultSerializationErrorPrinter : public ISerializationErrorReporter
	{
	public:
		DefaultSerializationErrorPrinter()
		{}

		virtual void reportTypeMissing(const StringBuf& serializationContextName, StringID missingTypeName) override
		{
			TRACE_WARNING("[DATA ERROR]: Unknown type '{}' used in serialization (context '{}')", missingTypeName, serializationContextName);
		}

		virtual void reportPropertyMissing(const StringBuf& serializationContextName, StringID classType, StringID missingPropertyName, StringID propertyType, bool handled) override
		{
			if (!handled)
				TRACE_WARNING("[DATA ERROR]: Unknown property '{} {}::{}' (context '{}')", propertyType, classType, missingPropertyName, serializationContextName);
		}

		virtual void reportPropertyTypeMissing(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID previousType, bool handled) override
		{
			if (!handled)
				TRACE_WARNING("[DATA ERROR]: Unknown type for property '{} {}::{}' (context '{}')", previousType, classType, propertyName, serializationContextName);
		}

		virtual void reportPropertyTypeChanged(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID previousType, StringID currentType, bool handled) override
		{
			if (!handled)
				TRACE_WARNING("[DATA ERROR]: Change property '{} {}::{}' from type '{}' (context '{}')", previousType, classType, propertyName, currentType, serializationContextName);
		}

		virtual void reportPropertyReadFailed(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID propertyType) override
		{
			TRACE_WARNING("[DATA ERROR]: Reading data for property '{} {}::{}' failed at (context '{}')", propertyType, classType, propertyName, serializationContextName);
		}

		virtual void reportPropertyEnumMissing(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID enumType, StringID enumOptionName) override
		{
			TRACE_WARNING("[DATA ERROR]: Missing enum option '{}' at property '{} {}::{}' (context '{}')", enumOptionName, enumType, classType, propertyName, serializationContextName);
		}

		virtual void reportPropertyBitflagMissing(const StringBuf& serializationContextName, StringID classType, StringID propertyName, StringID enumType, StringID enumOptionName) override
		{
			TRACE_WARNING("[DATA ERROR]: Missing flag value '{}' at property '{} {}::{}' (context '{}')", enumOptionName, enumType, classType, propertyName, serializationContextName);
		}
	};
}

ISerializationErrorReporter& ISerializationErrorReporter::DefaultErrorReporter()
{
	static prv::DefaultSerializationErrorPrinter thePrinter;
	return thePrinter;
}

//--

END_INFERNO_NAMESPACE()
