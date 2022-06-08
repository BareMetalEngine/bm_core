/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/object/include/rttiTypedMemory.h"
#include "testTypes.h"

BEGIN_INFERNO_NAMESPACE_EX(test)

//--

static const PrintFlags PRINT_FLAGS = { PrintFlagBit::NoHeader };

TEST(XMLWrite, SimpleInt)
{
    const int value = 42;
    const auto data = TypedMemory::Wrap(value);

    StringBuilder txt;
    ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
    EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

    EXPECT_STREQ("<data>42</data>", txt.c_str());
}

TEST(XMLWrite, SimpleFloat)
{
	const float value = 42.125f;
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>42.125</data>", txt.c_str());
}

TEST(XMLWrite, SimpleBool)
{
	const bool value = true;
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>true</data>", txt.c_str());
}

TEST(XMLWrite, SimpleString)
{
	const StringBuf value = "Ala ma kota";
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>Ala ma kota</data>", txt.c_str());
}

TEST(XMLWrite, SimpleStringWithEscapement)
{
	const StringBuf value = "Ala <ma> \"kota\"";
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>Ala &lt;ma&gt; &quot;kota&quot;</data>", txt.c_str());
}

TEST(XMLWrite, SimpleStringMultiline)
{
	const StringBuf value = "Ala ma\nkoty i psy";
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>Ala ma&#10;koty i psy</data>", txt.c_str());
}

TEST(XMLWrite, Compound)
{
	const Vector3 value(1, 2, 3);
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data><x>1</x><y>2</y><z>3</z></data>", txt.c_str());
}

TEST(XMLWrite, BigCompound)
{
	const Box value(Vector3(-1, -2, -3), Vector3(1, 2, 3));
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data><min><x>-1</x><y>-2</y><z>-3</z></min><max><x>1</x><y>2</y><z>3</z></max></data>", txt.c_str());
}

TEST(XMLWrite, Array)
{
	const Array<int> ar = { 1,2,3 };
	const auto data = TypedMemory::Wrap(ar);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<array><element>1</element><element>2</element><element>3</element></array>", txt.c_str());
}

TEST(XMLWrite, ArrayCompound)
{
	const Array<Vector3> ar = { Vector3(1,2,3), Vector3(4,5,6) };
	const auto data = TypedMemory::Wrap(ar);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<array><element><x>1</x><y>2</y><z>3</z></element><element><x>4</x><y>5</y><z>6</z></element></array>", txt.c_str());
}

TEST(XMLWrite, Enumeration)
{
	const TestEnum val = TestEnum::Second;
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>Second</data>", txt.c_str());
}

TEST(XMLWrite, EnumerationMissing)
{
	const TestEnum val = (TestEnum)42;
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>42</data>", txt.c_str());
}

TEST(XMLWrite, BitfieldEmpty)
{
	const TestFlags val;
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data/>", txt.c_str());
}

TEST(XMLWrite, BitfieldSingle)
{
	const TestFlags val = {TestFlagBit::Second};
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>Second</data>", txt.c_str());
}

TEST(XMLWrite, BitfieldTwoFlags)
{
	const TestFlags val = { TestFlagBit::First, TestFlagBit::Third};
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>First;Third</data>", txt.c_str());
}

TEST(XMLWrite, BitfieldMissingFlag)
{
	const TestFlags val = { (TestFlagBit)5 };
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>5</data>", txt.c_str());
}

TEST(XMLWrite, BitfieldFlagMixed)
{
	const TestFlags val = { (TestFlagBit)5, TestFlagBit::Second };
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>Second;5</data>", txt.c_str());
}

TEST(XMLWrite, CustomType)
{
	const GUID val = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>{01020304-0506-0708-0910-111213141516}</data>", txt.c_str());
}

TEST(XMLWrite, NullPointer)
{
	const ObjectPtr ptr;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object>null</object>", txt.c_str());
}

TEST(XMLWrite, EmptyObject)
{
	const ObjectPtr ptr = RefNew<TestObject>();;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object class=\"test.TestObject\"/>", txt.c_str());
}

TEST(XMLWrite, ObjectWithSimpleProp)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->simple = 42.0f;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object class=\"test.TestObject\"><simple>42</simple></object>", txt.c_str());
}

TEST(XMLWrite, NestedObject)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->ptr = RefNew<TestObject>();
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object class=\"test.TestObject\"><ptr class=\"test.TestObject\"/></object>", txt.c_str());
}

TEST(XMLWrite, IndexedObject)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	auto second = RefNew<TestObject>();
	second->simple = 42.0f;
	ptr->arrayPtrs.pushBack(second);
	ptr->arrayPtrs.pushBack(second);
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object class=\"test.TestObject\"><arrayPtrs><element class=\"test.TestObject\" id=\"1\"><simple>42</simple></element><element refId=\"1\"/></arrayPtrs></object>", txt.c_str());
}

TEST(XMLWrite, KnownWeakPtrSurvives)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->ptr = RefNew<TestObject>();
	ptr->weakPtr = ptr->ptr;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object class=\"test.TestObject\"><ptr class=\"test.TestObject\" id=\"1\"/><weakPtr refId=\"1\"/></object>", txt.c_str());
}

TEST(XMLWrite, UnknownWeakPtrGetsNulled)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->ptr = RefNew<TestObject>();
	const auto other = RefNew<TestObject>();
	ptr->weakPtr = other;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<object class=\"test.TestObject\"><ptr class=\"test.TestObject\"/><weakPtr>null</weakPtr></object>", txt.c_str());
}

TEST(XMLWrite, ResourceEmpty)
{
	ResourceRef<TestResource> ptr;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>null</data>", txt.c_str());
}

TEST(XMLWrite, ResourceExternal)
{
	auto resourceID = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	auto resourcePromise = ResourcePromise::CreateEmptyPromise(resourceID, TestResource::GetStaticClass());

	ResourceRef<TestResource> ptr = BaseReference(resourcePromise);
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data class=\"test.TestResource\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>", txt.c_str());
}

TEST(XMLWrite, ResourceInternal)
{
	auto resPtr = RefNew<TestResource>();

	ResourceRef<TestResource> ptr = BaseReference(resPtr);
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data class=\"test.TestResource\"/>", txt.c_str());
}

TEST(XMLWrite, ResourceInternalDupllicated)
{
	auto resPtr = RefNew<TestResource>();

	Array<ResourceRef<TestResource>> ar;
	ar.pushBack(BaseReference(resPtr));
	ar.pushBack(BaseReference(resPtr));

	const auto data = TypedMemory::Wrap(ar);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<array><element class=\"test.TestResource\" id=\"1\"/><element refId=\"1\"/></array>", txt.c_str());
}

TEST(XMLWrite, BufferEmpty)
{
	Buffer dataBuf;

	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data/>", txt.c_str());
}

TEST(XMLWrite, BufferSimple)
{
	uint8_t bytes[3] = { 'a','b','c' };
	Buffer dataBuf = Buffer::CreateFromCopy(MainPool(), bytes, 3);

	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>YWJj</data>", txt.c_str());
}

TEST(XMLWrite, AsyncBufferEmpty)
{
	AsyncFileBuffer dataBuf;
	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data/>", txt.c_str());
}

TEST(XMLWrite, AsyncBufferSimple)
{
	uint8_t bytes[3] = { 'a','b','c' };

	AsyncFileBuffer dataBuf;
	dataBuf.setup(Buffer::CreateFromCopy(MainPool(), bytes, 3));

	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

	EXPECT_STREQ("<data>YWJj</data>", txt.c_str());
}

TEST(XMLWrite, Save100)
{
	bm::Random r;
	TestLayerGenerationSettings settings;
	settings.maxChildrenPerEntity = 1;
	settings.maxComponentsPerEntity = 2;
	settings.maxEntityDepth = 1;
	settings.maxLinksPerEntity = 0;
	settings.numEntities = 100;

	auto layer = GenerateTestLayer(r, settings);
	{
		ScopeTimer timer;

		const auto data = TypedMemory::Wrap(layer);

		StringBuilder txt;
		ObjectSavingContext ctx;
		ctx.textPrintFlags = PRINT_FLAGS;
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

		TRACE_INFO("Saving time 100: {}", timer);
		EXPECT_TRUE(!txt.empty());
	}
}

TEST(XMLWrite, Save1000)
{
	bm::Random r;
	TestLayerGenerationSettings settings;
	settings.maxChildrenPerEntity = 2;
	settings.maxComponentsPerEntity = 4;
	settings.maxEntityDepth = 2;
	settings.maxLinksPerEntity = 0;
	settings.numEntities = 1000;

	auto layer = GenerateTestLayer(r, settings);
	{
		ScopeTimer timer;

		const auto data = TypedMemory::Wrap(layer);

		StringBuilder txt;
		ObjectSavingContext ctx;
		ctx.textPrintFlags = PRINT_FLAGS;
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

		TRACE_INFO("Saving time 1K: {}", timer);
		EXPECT_TRUE(!txt.empty());
	}
}

#ifdef BUILD_RELEASE
TEST(XMLWrite, Save10K)
{
	bm::Random r;
	TestLayerGenerationSettings settings;
	settings.maxChildrenPerEntity = 2;
	settings.maxComponentsPerEntity = 6;
	settings.maxEntityDepth = 2;
	settings.maxLinksPerEntity = 0;
	settings.numEntities = 10000;

	auto layer = GenerateTestLayer(r, settings);
	//for (;;)
	{
		/*
		{
			TRACE_WARNING("Abount to start");
			Thread::Sleep(2000);
		}*/
		{
			ScopeTimer timer;

			const auto data = TypedMemory::Wrap(layer);

			StringBuilder txt;
			ObjectSavingContext ctx;
			ctx.textPrintFlags = PRINT_FLAGS;
			EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, data, txt));

			TRACE_WARNING("Saving time 10K: {}", timer);
			EXPECT_TRUE(!txt.empty());
		}
		/*{
			TRACE_WARNING("Just finished");
			IPool::PrintPools(TRACE_STREAM_WARNING());
			Thread::Sleep(2000);
		}*/
	}
}
#endif

//--

END_INFERNO_NAMESPACE_EX(test)
