/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "testTypes.h"
#include "bm/core/object/include/rttiTypedMemory.h"
#include "bm/core/object/include/serializationStream.h"

BEGIN_INFERNO_NAMESPACE_EX(test)

static const PrintFlags PRINT_FLAGS = { PrintFlagBit::NoHeader };

//--

TEST(XMLRead, SimpleInt)
{
	StringView txt = "<data>42</data>";

	int value = 0;
    auto data = TypedMemory::Wrap(value);

    ObjectLoadingContext ctx;
    EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(42, value);
}

TEST(XMLRead, SimpleIntParsingFailed)
{
	StringView txt = "<data>x</data>";

	int value = 0;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	TestErrorReporter err;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(0, value);
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, SimpleFloat)
{
	StringView txt = "<data>42.125</data>";

	float value = 0;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(42.125f, value);
}

TEST(XMLRead, SimpleBool)
{
	StringView txt = "<data>true</data>";

	bool value = false;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(true, value);
}

TEST(XMLRead, SimpleBoolFast)
{
	StringView txt = "<data>false</data>";

	bool value = true;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(false, value);
}

TEST(XMLRead, SimpleBoolParsesFromInt)
{
	StringView txt = "<data>1</data>";

	bool value = false;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(true, value);
}

TEST(XMLRead, SimpleBoolParsesFromIntFalse)
{
	StringView txt = "<data>0</data>";

	bool value = true;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(false, value);
}

TEST(XMLRead, SimpleBoolFailsToParseFromCrap)
{
	StringView txt = "<data>crap</data>";

	bool value = false;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(false, value);
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, SimpleString)
{
	StringView txt = "<data>Ala ma kota</data>";

	StringBuf value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_STREQ("Ala ma kota", value.c_str());
}

TEST(XMLRead, SimpleStringWithEscapement)
{
	StringView txt = "<data>Ala &lt;ma&gt; &quot;kota&quot;</data>";

	StringBuf value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_STREQ("Ala <ma> \"kota\"", value.c_str());
}

TEST(XMLRead, SimpleStringMultiline)
{
	StringView txt = "<data>Ala ma&#10;koty i psy</data>";

	StringBuf value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_STREQ("Ala ma\nkoty i psy", value.c_str());
}

TEST(XMLRead, Compound)
{
	StringView txt = "<data><x>1</x><y>2</y><z>3</z></data>";

	Vector3 value(0, 0, 0);
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(1.0f, value.x);
	EXPECT_EQ(2.0f, value.y);
	EXPECT_EQ(3.0f, value.z);
}

#if 0
TEST(XMLRead, CompoundReportsMissingProperty)
{
	StringView txt = "<data><w>1</w></data>";

	Vector3 value(0, 0, 0);
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(1, err.numErrors);
}
#endif

TEST(XMLRead, BigCompound)
{
	StringView txt = "<data><min><x>-1</x><y>-2</y><z>-3</z></min><max><x>1</x><y>2</y><z>3</z></max></data>";

	Box value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(-1.0f, value.min.x);
	EXPECT_EQ(-2.0f, value.min.y);
	EXPECT_EQ(-3.0f, value.min.z);
	EXPECT_EQ(1.0f, value.max.x);
	EXPECT_EQ(2.0f, value.max.y);
	EXPECT_EQ(3.0f, value.max.z);
}

TEST(XMLRead, Array)
{
	StringView txt = "<array><element>1</element><element>2</element><element>3</element></array>";

	Array<int> value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(3, value.size());
	EXPECT_EQ(1, value[0]);
	EXPECT_EQ(2, value[1]);
	EXPECT_EQ(3, value[2]);
}

TEST(XMLRead, StaticArray)
{
	StringView txt = "<array><element>1</element><element>2</element><element>3</element></array>";

	int value[3];
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(1, value[0]);
	EXPECT_EQ(2, value[1]);
	EXPECT_EQ(3, value[2]);
}

TEST(XMLRead, StaticArrayDoesNotChangeExisting)
{
	StringView txt = "<array><element>1</element><element>2</element></array>";

	int value[3] = {5,10,15};
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(1, value[0]);
	EXPECT_EQ(2, value[1]);
	EXPECT_EQ(15, value[2]);
}

TEST(XMLRead, StaticArrayReportsToManyElements)
{
	StringView txt = "<array><element>1</element><element>2</element><element>3</element><element>4</element></array>";

	int value[3];
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(1, value[0]);
	EXPECT_EQ(2, value[1]);
	EXPECT_EQ(3, value[2]);
	EXPECT_EQ(2, err.numErrors);
}

TEST(XMLRead, ArrayCompound)
{
	StringView txt = "<array><element><x>1</x><y>2</y><z>3</z></element><element><x>4</x><y>5</y><z>6</z></element></array>";

	Array<Vector3> value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(2, value.size());
	EXPECT_EQ(1.0f, value[0].x);
	EXPECT_EQ(2.0f, value[0].y);
	EXPECT_EQ(3.0f, value[0].z);
	EXPECT_EQ(4.0f, value[1].x);
	EXPECT_EQ(5.0f, value[1].y);
	EXPECT_EQ(6.0f, value[1].z);
}

TEST(XMLRead, Enumeration)
{
	StringView txt = "<data>Second</data>";

	TestEnum value = (TestEnum)0;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(TestEnum::Second, value);
}

TEST(XMLRead, EnumerationValueTrimmed)
{
	StringView txt = "<data> Second </data>";

	TestEnum value = (TestEnum)0;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(TestEnum::Second, value);
}

TEST(XMLRead, EnumerationMissing)
{
	StringView txt = "<data>42</data>";

	TestEnum value = (TestEnum)0;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ((TestEnum)42, value);
}

TEST(XMLRead, EnumerationReportsInvalid)
{
	StringView txt = "<data>Crap</data>";

	TestEnum value = TestEnum::Second;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(TestEnum::Second, value);
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, BitfieldEmpty)
{
	StringView txt = "<data/>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(0, value.rawValue());
}

TEST(XMLRead, BitfieldSingle)
{
	StringView txt = "<data>Second</data>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_TRUE(value.test(TestFlagBit::Second));
	EXPECT_EQ(2, value.rawValue());
}

TEST(XMLRead, BitfieldTwoFlags)
{
	StringView txt = "<data>First;Third</data>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_TRUE(value.test(TestFlagBit::First));
	EXPECT_FALSE(value.test(TestFlagBit::Second));
	EXPECT_TRUE(value.test(TestFlagBit::Third));
	EXPECT_EQ(5, value.rawValue());
}

TEST(XMLRead, BitfieldMissingFlag)
{
	StringView txt = "<data>5</data>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_FALSE(value.test(TestFlagBit::First));
	EXPECT_FALSE(value.test(TestFlagBit::Second));
	EXPECT_FALSE(value.test(TestFlagBit::Third));
	EXPECT_EQ(32, value.rawValue());
}

TEST(XMLRead, BitfieldFlagMixed)
{
	StringView txt = "<data>5;Second</data>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_FALSE(value.test(TestFlagBit::First));
	EXPECT_TRUE(value.test(TestFlagBit::Second));
	EXPECT_FALSE(value.test(TestFlagBit::Third));
	EXPECT_EQ(34, value.rawValue());
}

TEST(XMLRead, BitfieldFlagMixedOlderDoesNotMatter)
{
	StringView txt = "<data>Second;5</data>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_FALSE(value.test(TestFlagBit::First));
	EXPECT_TRUE(value.test(TestFlagBit::Second));
	EXPECT_FALSE(value.test(TestFlagBit::Third));
	EXPECT_EQ(34, value.rawValue());
}

TEST(XMLRead, BitfieldFlagValueTrimmed)
{
	StringView txt = "<data> Second </data>";

	TestFlags value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_TRUE(value.test(TestFlagBit::Second));
	EXPECT_EQ(2, value.rawValue());
}

TEST(XMLRead, BitfieldReportsError)
{
	StringView txt = "<data>Crap</data>";

	TestFlags value = TestFlagBit::Second;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_FALSE(value.test(TestFlagBit::Second)); // value should be unset
	EXPECT_EQ(0, value.rawValue());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, CustomType)
{
	StringView txt = "<data>{01020304-0506-0708-0910-111213141516}</data>";

	GUID value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));

	const GUID testVal = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	EXPECT_EQ(testVal.data()[0], value.data()[0]);
	EXPECT_EQ(testVal.data()[1], value.data()[1]);
	EXPECT_EQ(testVal.data()[2], value.data()[2]);
	EXPECT_EQ(testVal.data()[3], value.data()[3]);
}

TEST(XMLRead, CustomTypeReportsFailure)
{
	StringView txt = "<data>{010203004-crap}</data>";

	GUID value;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, NullPointer)
{
	StringView txt = "<object>null</object>";

	ObjectPtr value = RefNew<TestObject>();
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	EXPECT_EQ(nullptr, value.get());
}

TEST(XMLRead, EmptyObject)
{
	StringView txt = "<object class=\"test.TestObject\"/>";

	ObjectPtr value = nullptr;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_NE(nullptr, value.get());
	EXPECT_STREQ("test.TestObject", value->cls().name().c_str());
}

TEST(XMLRead, DiffferentTypeDoesNotBind)
{
	StringView txt = "<object class=\"test.TestResource\"/>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(nullptr, value.get());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, UnknownTypeDoesNotBind)
{
	StringView txt = "<object class=\"crap\"/>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	EXPECT_EQ(nullptr, value.get());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, ObjectWithSimpleProp)
{
	StringView txt = "<object class=\"test.TestObject\"><simple>42</simple></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_NE(nullptr, value.get());
	EXPECT_STREQ("test.TestObject", value->cls().name().c_str());
	EXPECT_EQ(42.0f, value->simple);
}

TEST(XMLRead, NestedObject)
{
	StringView txt = "<object class=\"test.TestObject\"><ptr class=\"test.TestObject\"/></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_NE(nullptr, value.get());
	EXPECT_STREQ("test.TestObject", value->cls().name().c_str());
	ASSERT_NE(nullptr, value->ptr.get());
	EXPECT_STREQ("test.TestObject", value->ptr->cls().name().c_str());
}

TEST(XMLRead, IndexedObject)
{
	StringView txt = "<object class=\"test.TestObject\"><arrayPtrs><element class=\"test.TestObject\" id=\"1\"><simple>42</simple></element><element refId=\"1\"/></arrayPtrs></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_NE(nullptr, value.get());
	EXPECT_STREQ("test.TestObject", value->cls().name().c_str());
	ASSERT_EQ(2, value->arrayPtrs.size());
	ASSERT_EQ(value->arrayPtrs[0].get(), value->arrayPtrs[1].get());
}

TEST(XMLRead, RedefinedIndexedObjectIngored)
{
	StringView txt = "<object class=\"test.TestObject\"><arrayPtrs><element class=\"test.TestObject\" id=\"1\"><simple>42</simple></element><element class=\"test.TestObject\" id=\"1\"/></arrayPtrs></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	ASSERT_NE(nullptr, value.get());
	EXPECT_STREQ("test.TestObject", value->cls().name().c_str());
	ASSERT_EQ(2, value->arrayPtrs.size());
	ASSERT_NE(value->arrayPtrs[0].get(), value->arrayPtrs[1].get());
	EXPECT_EQ(42.0f, value->arrayPtrs[0]->simple);
	EXPECT_EQ(0.0f, value->arrayPtrs[1]->simple);	
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, IncompleteObjectDefinition)
{
	StringView txt = "<object></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	ASSERT_EQ(nullptr, value.get());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, MissingObjectNulled)
{
	StringView txt = "<object class=\"test.TestObject\"><ptr><element refId=\"1\"/></ptr></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));
	ASSERT_NE(nullptr, value.get());
	ASSERT_EQ(nullptr, value->ptr.get());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, KnownWeakPtrSurvives)
{
	StringView txt = "<object class=\"test.TestObject\"><ptr class=\"test.TestObject\" id=\"1\"/><weakPtr refId=\"1\"/></object>";

	RefPtr<TestObject> value = nullptr;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_NE(nullptr, value.get());
	EXPECT_STREQ("test.TestObject", value->cls().name().c_str());
	ASSERT_NE(nullptr, value->ptr.get());
	ASSERT_NE(nullptr, value->weakPtr.unsafe());
	ASSERT_EQ(value->ptr.get(), value->weakPtr.unsafe());
}

TEST(XMLRead, ResourceEmpty)
{
	StringView txt = "<data>null</data>";

	ResourceRef<TestResource> value = BaseReference(RefNew<TestResource>());
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(nullptr, value.resolveResource());	
}

TEST(XMLRead, ResourceIncompleteHandled)
{
	StringView txt = "<data></data>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));

	EXPECT_EQ(GUID(), value.resolveId());
	EXPECT_EQ(ClassType(), value.resolveClass());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, ResourceInvalidClassHandled)
{
	StringView txt = "<data class=\"crap\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));

	EXPECT_EQ(GUID(), value.resolveId());
	EXPECT_EQ(ClassType(), value.resolveClass());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, ResourceNonResourceClassHandled)
{
	StringView txt = "<data class=\"test.TestObject\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));

	EXPECT_EQ(GUID(), value.resolveId());
	EXPECT_EQ(ClassType(), value.resolveClass());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, ResourceInvalidGuidHandled)
{
	StringView txt = "<data class=\"test.TestResource\" guid=\"{crap}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	TestErrorReporter err;
	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt, err));

	EXPECT_EQ(GUID(), value.resolveId());
	EXPECT_EQ(ClassType(), value.resolveClass());
	EXPECT_EQ(1, err.numErrors);
}

TEST(XMLRead, ResourceExternalPromiseCreated)
{
	StringView txt = "<data class=\"test.TestResource\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	
	auto resourceID = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	EXPECT_EQ(resourceID, value.resolveId());
	EXPECT_EQ(TestResource::GetStaticClass(), value.resolveClass());
	EXPECT_EQ(nullptr, value.resolveResource());
}

TEST(XMLRead, ResourceExternalPromiseReported)
{
	StringView txt = "<data class=\"test.TestResource\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	Array<ResourcePromisePtr> promises;

	ObjectLoadingContext ctx;
	ctx.resourcePromises = &promises;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));

	auto resourceID = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	ASSERT_EQ(1, promises.size());

	const auto& promise = promises[0];
	EXPECT_EQ(resourceID, promise->id());
}

TEST(XMLRead, ResourceExternalPromiseNotFullfilled)
{
	StringView txt = "<data class=\"test.TestResource\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	Array<ResourcePromisePtr> promises;

	ObjectLoadingContext ctx;
	ctx.resourcePromises = &promises;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));

	auto resourceID = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	EXPECT_EQ(1, promises.size());

	for (const auto& promise : promises)
	{
		ASSERT_FALSE(promise->fullfilled());
	}
}

TEST(XMLRead, ResourceExternalPromiseConnected)
{
	StringView txt = "<data class=\"test.TestResource\" guid=\"{01020304-0506-0708-0910-111213141516}\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	Array<ResourcePromisePtr> promises;

	ObjectLoadingContext ctx;
	ctx.resourcePromises = &promises;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));

	auto resourceID = GUID(0x04030201, 0x08070605, 0x12111009, 0x16151413);
	ASSERT_EQ(1, promises.size());

	SerializationResourceKey key;
	key.id = resourceID;
	key.className = "test.TestResource"_id;

	const auto& promise = promises[0];
	EXPECT_EQ(resourceID, promise->id());

	auto resource = RefNew<TestResource>();
	promise->fulfill(resource);

	EXPECT_EQ(value.resolveResource(), resource);
}

TEST(XMLRead, ResourceInternal)
{
	StringView txt = "<data class=\"test.TestResource\"/>";

	ResourceRef<TestResource> value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_NE(nullptr, value.resolveResource());
}

TEST(XMLRead, ResourceInternalDupllicated)
{
	StringView txt = "<array><element class=\"test.TestResource\" id=\"1\"/><element refId=\"1\"/></array>";

	Array<ResourceRef<TestResource>> value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(2, value.size());
	ASSERT_NE(nullptr, value[0].resolveResource().get());
	ASSERT_EQ(value[0].resolveResource().get(), value[0].resolveResource().get());
}

TEST(XMLRead, BufferEmpty)
{
	StringView txt = "<data/>";

	Buffer value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(0, value.size());
}

TEST(XMLRead, BufferSimple)
{
	StringView txt = "<data>YWJj</data>";

	Buffer value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(3, value.size());
	EXPECT_EQ('a', value.data()[0]);
	EXPECT_EQ('b', value.data()[1]);
	EXPECT_EQ('c', value.data()[2]);
}

TEST(XMLRead, AsyncBufferEmpty)
{
	StringView txt = "<data/>";

	AsyncFileBuffer value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_TRUE(value.empty());
}

TEST(XMLRead, AsyncBufferSimple)
{
	StringView txt = "<data>YWJj</data>";

	AsyncFileBuffer value;
	auto data = TypedMemory::Wrap(value);

	ObjectLoadingContext ctx;
	EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, data, txt));
	ASSERT_EQ(3, value.size());

	auto data2 = value.load(NoTask(), MainPool());
	ASSERT_EQ(3, data2.size());
	EXPECT_EQ('a', data2.data()[0]);
	EXPECT_EQ('b', data2.data()[1]);
	EXPECT_EQ('c', data2.data()[2]);
}

//--

TEST(XMLCheck, SaveAndLoad1PrecissionFloats)
{
	bm::Random r;

	Array<float> arr;
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.unit());
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.range(-1024.0f, -1024.0f));
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.range(0.0f, 0.000000001f));
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.range(10000000.0f, 100000000000000.0f));

	StringBuilder txt;
	{
		ObjectSavingContext ctx;
		ctx.textPrintFlags = PRINT_FLAGS;
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, TypedMemory::Wrap(arr), txt));
	}

	Array<float> arr2;
	{
		ObjectLoadingContext ctx;
		EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, TypedMemory::Wrap(arr2), txt));
	}

	ASSERT_EQ(arr2.size(), arr.size());
	for (uint32_t i = 0; i < arr.size(); ++i)
	{
		const float a = arr[i];
		const float b = arr2[i];
		EXPECT_FLOAT_EQ(a, b);
		const uint32_t ua = (uint32_t&)a;
		const uint32_t ub = (uint32_t&)b;
		EXPECT_EQ(ua, ub);
	}
}

TEST(XMLCheck, SaveAndLoad1PrecissionDoubles)
{
	bm::Random r;

	Array<double> arr;
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.unitDouble());
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.rangeDouble(-1024.0f, -1024.0f));
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.rangeDouble(0.0f, 0.000000000001));
	for (uint32_t i = 0; i < 1000; i++)
		arr.pushBack(r.rangeDouble(10000000.0, 100000000000000.0));

	StringBuilder txt;
	{
		ObjectSavingContext ctx;
		ctx.textPrintFlags = PRINT_FLAGS;
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, TypedMemory::Wrap(arr), txt));
	}

	Array<double> arr2;
	{
		ObjectLoadingContext ctx;
		EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, TypedMemory::Wrap(arr2), txt));
	}

	ASSERT_EQ(arr2.size(), arr.size());
	for (uint32_t i = 0; i < arr.size(); ++i)
	{
		const float a = arr[i];
		const float b = arr2[i];
		EXPECT_FLOAT_EQ(a, b);
		const uint64_t ua = (uint64_t&)a;
		const uint64_t ub = (uint64_t&)b;
		EXPECT_EQ(ua, ub);
	}
}

TEST(XMLCheck, SaveAndLoad1000)
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

		StringBuilder txt;
		ObjectSavingContext ctx;
		ctx.textPrintFlags = PRINT_FLAGS;
		EXPECT_TRUE(TypedMemory::SaveData(SerializationFormat::XML, ctx, TypedMemory::Wrap(layer), txt));

		TRACE_INFO("Saving time 1K: {} ({})", timer, MemSize(txt.length()));
		EXPECT_TRUE(!txt.empty());

		// load
		RefPtr<TestLayer> layer2;
		{
			ScopeTimer timer2;

			ObjectLoadingContext ctx;
			EXPECT_TRUE(TypedMemory::LoadData(SerializationFormat::XML, ctx, TypedMemory::Wrap(layer2), txt));
			ASSERT_NE(nullptr, layer2);

			TRACE_INFO("Loading time 1K: {}", timer2);

			layer->Compare(layer2);
		}
	}
}


END_INFERNO_NAMESPACE_EX(test)
