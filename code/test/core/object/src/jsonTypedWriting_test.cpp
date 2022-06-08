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

TEST(JSONWrite, SimpleInt)
{
    const int value = 42;
    const auto data = TypedMemory::Wrap(value);

    StringBuilder txt;
    ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
    EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

    EXPECT_STREQ("\"42\"", txt.c_str());
}

TEST(JSONWrite, SimpleFloat)
{
	const float value = 42.125f;
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"42.125\"", txt.c_str());
}

TEST(JSONWrite, SimpleBool)
{
	const bool value = true;
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"true\"", txt.c_str());
}

TEST(JSONWrite, SimpleString)
{
	const StringBuf value = "Ala ma kota";
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"Ala ma kota\"", txt.c_str());
}

TEST(JSONWrite, SimpleStringWithEscapement)
{
	const StringBuf value = "Ala <ma> \"kota\"";
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"Ala <ma> \\\"kota\\\"\"", txt.c_str());
}

TEST(JSONWrite, SimpleStringMultiline)
{
	const StringBuf value = "Ala ma\nkoty i psy";
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"Ala ma\\nkoty i psy\"", txt.c_str());
}

TEST(JSONWrite, Compound)
{
	const Vector3 value(1, 2, 3);
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"x\": \"1\", \"y\": \"2\", \"z\": \"3\"}", txt.c_str());
}

TEST(JSONWrite, BigCompound)
{
	const Box value(Vector3(-1, -2, -3), Vector3(1, 2, 3));
	const auto data = TypedMemory::Wrap(value);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"min\": {\"x\": \"-1\", \"y\": \"-2\", \"z\": \"-3\"}, \"max\": {\"x\": \"1\", \"y\": \"2\", \"z\": \"3\"}}", txt.c_str());
}

TEST(JSONWrite, Array)
{
	const Array<int> ar = { 1,2,3 };
	const auto data = TypedMemory::Wrap(ar);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("[\"1\", \"2\", \"3\"]", txt.c_str());
}

TEST(JSONWrite, ArrayCompound)
{
	const Array<Vector3> ar = { Vector3(1,2,3), Vector3(4,5,6) };
	const auto data = TypedMemory::Wrap(ar);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("[{\"x\": \"1\", \"y\": \"2\", \"z\": \"3\"}, {\"x\": \"4\", \"y\": \"5\", \"z\": \"6\"}]", txt.c_str());
}

TEST(JSONWrite, Enumeration)
{
	const TestEnum val = TestEnum::Second;
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"Second\"", txt.c_str());
}

TEST(JSONWrite, EnumerationMissing)
{
	const TestEnum val = (TestEnum)42;
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"42\"", txt.c_str());
}

TEST(JSONWrite, BitfieldEmpty)
{
	const TestFlags val;
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"\"", txt.c_str());
}

TEST(JSONWrite, BitfieldSingle)
{
	const TestFlags val = {TestFlagBit::Second};
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"Second\"", txt.c_str());
}

TEST(JSONWrite, BitfieldTwoFlags)
{
	const TestFlags val = { TestFlagBit::First, TestFlagBit::Third};
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"First;Third\"", txt.c_str());
}

TEST(JSONWrite, BitfieldMissingFlag)
{
	const TestFlags val = { (TestFlagBit)5 };
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"5\"", txt.c_str());
}

TEST(JSONWrite, BitfieldFlagMixed)
{
	const TestFlags val = { (TestFlagBit)5, TestFlagBit::Second };
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"Second;5\"", txt.c_str());
}

TEST(JSONWrite, CustomType)
{
	const GUID val = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	const auto data = TypedMemory::Wrap(val);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"{01020304-0506-0708-0910-111213141516}\"", txt.c_str());
}

TEST(JSONWrite, NullPointer)
{
	const ObjectPtr ptr;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"null\"", txt.c_str());
}

TEST(JSONWrite, EmptyObject)
{
	const ObjectPtr ptr = RefNew<TestObject>();;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestObject\"}", txt.c_str());
}

TEST(JSONWrite, ObjectWithSimpleProp)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->simple = 42.0f;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestObject\", \"simple\": \"42\"}", txt.c_str());
}

TEST(JSONWrite, NestedObject)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->ptr = RefNew<TestObject>();
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestObject\", \"ptr\": {\"#class\": \"test.TestObject\"}}", txt.c_str());
}

TEST(JSONWrite, IndexedObject)
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
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestObject\", \"arrayPtrs\": [{\"#class\": \"test.TestObject\", \"#id\": \"1\", \"simple\": \"42\"}, \"#ref(1)\"]}", txt.c_str());
}

TEST(JSONWrite, KnownWeakPtrSurvives)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->ptr = RefNew<TestObject>();
	ptr->weakPtr = ptr->ptr;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestObject\", \"ptr\": {\"#class\": \"test.TestObject\", \"#id\": \"1\"}, \"weakPtr\": \"#ref(1)\"}", txt.c_str());
}

TEST(JSONWrite, UnknownWeakPtrGetsNulled)
{
	const RefPtr<TestObject> ptr = RefNew<TestObject>();
	ptr->ptr = RefNew<TestObject>();
	const auto other = RefNew<TestObject>();
	ptr->weakPtr = other;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestObject\", \"ptr\": {\"#class\": \"test.TestObject\"}, \"weakPtr\": \"null\"}", txt.c_str());
}

TEST(JSONWrite, ResourceEmpty)
{
	ResourceRef<TestResource> ptr;
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"null\"", txt.c_str());
}

TEST(JSONWrite, ResourceExternal)
{
	auto resourceID = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	auto resourcePromise = ResourcePromise::CreateEmptyPromise(resourceID, TestResource::GetStaticClass());

	ResourceRef<TestResource> ptr = BaseReference(resourcePromise);
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"test.TestResource:{01020304-0506-0708-0910-111213141516}\"", txt.c_str());
}

TEST(JSONWrite, ResourceInternal)
{
	auto resPtr = RefNew<TestResource>();

	ResourceRef<TestResource> ptr = BaseReference(resPtr);
	const auto data = TypedMemory::Wrap(ptr);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("{\"#class\": \"test.TestResource\"}", txt.c_str());
}

TEST(JSONWrite, ResourceInternalDupllicated)
{
	auto resPtr = RefNew<TestResource>();

	Array<ResourceRef<TestResource>> ar;
	ar.pushBack(BaseReference(resPtr));
	ar.pushBack(BaseReference(resPtr));

	const auto data = TypedMemory::Wrap(ar);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("[{\"#class\": \"test.TestResource\", \"#id\": \"1\"}, \"#ref(1)\"]", txt.c_str());
}

TEST(JSONWrite, BufferEmpty)
{
	Buffer dataBuf;

	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"\"", txt.c_str());
}

TEST(JSONWrite, BufferSimple)
{
	uint8_t bytes[3] = { 'a','b','c' };
	Buffer dataBuf = Buffer::CreateFromCopy(MainPool(), bytes, 3);

	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"YWJj\"", txt.c_str());
}

TEST(JSONWrite, AsyncBufferEmpty)
{
	AsyncFileBuffer dataBuf;
	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"\"", txt.c_str());
}

TEST(JSONWrite, AsyncBufferSimple)
{
	uint8_t bytes[3] = { 'a','b','c' };

	AsyncFileBuffer dataBuf;
	dataBuf.setup(Buffer::CreateFromCopy(MainPool(), bytes, 3));

	const auto data = TypedMemory::Wrap(dataBuf);

	StringBuilder txt;
	ObjectSavingContext ctx;
	ctx.textPrintFlags = PRINT_FLAGS;
	EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

	EXPECT_STREQ("\"YWJj\"", txt.c_str());
}

TEST(JSONWrite, Save100)
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
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

		TRACE_INFO("Saving time 100: {}", timer);
		EXPECT_TRUE(!txt.empty());
	}
}

TEST(JSONWrite, Save1000)
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
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

		TRACE_INFO("Saving time 1K: {}", timer);
		EXPECT_TRUE(!txt.empty());
	}
}

#ifdef BUILD_RELEASE
TEST(JSONWrite, Save10K)
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
			EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::Json, ctx, data, txt));

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
