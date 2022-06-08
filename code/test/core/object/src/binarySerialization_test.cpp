/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringBuf.h"
#include "bm/core/object/include/object.h"
#include "bm/core/math/include/mathRandom.h"
#include "bm/core/object/include/resourceReference.h"
#include "bm/core/object/include/resource.h"
#include "bm/core/memory/include/pool.h"
#include "bm/core/object/include/resourcePromise.h"
#include "bm/core/object/include/asyncBuffer.h"
#include "testTypes.h"

BEGIN_INFERNO_NAMESPACE_EX(test)

//--

TEST(BinarySerialization, ObjectSaved)
{
    auto ptr = RefNew<TestObject>();

    ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));

    ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
    EXPECT_NE(nullptr, ptr);
}

TEST(BinarySerialization, CompareEmptyObject)
{
	auto ptr = RefNew<TestObject>();

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
    ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareTrivialType)
{
	auto ptr = RefNew<TestObject>();
    ptr->trivial = true;

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareSimple)
{
	auto ptr = RefNew<TestObject>();
	ptr->simple = 42.0f;

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareString)
{
	auto ptr = RefNew<TestObject>();
	ptr->txt = "Ala ma kota, kota ma ale";

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareCompound)
{
	auto ptr = RefNew<TestObject>();
    ptr->compound.y = -2.0f;
    ptr->compound.z = 3.0f;

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareCompoundEx)
{
	auto ptr = RefNew<TestObject>();
	ptr->compoundEx.min.x = -1.0f;
    ptr->compoundEx.min.y = -2.0f;
    ptr->compoundEx.min.z = -3.0f;	
	ptr->compoundEx.max.x = 1.0f;
	ptr->compoundEx.max.y = 2.0f;
	ptr->compoundEx.max.z = 3.0f;

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareArraySimple)
{
	auto ptr = RefNew<TestObject>();
    ptr->arraySimple.pushBack(1.0f);
    ptr->arraySimple.pushBack(2.0f);
    ptr->arraySimple.pushBack(3.0f);
    ptr->arraySimple.pushBack(4.0f);
    ptr->arraySimple.pushBack(5.0f);

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, ComparePreciseNumericFloat)
{
	Random r;

	auto ptr = RefNew<TestObject>();
	for (uint32_t i = 0; i < 1000; ++i)
	{
		ptr->arrayFloat.pushBack(r.range(-1024, 1024));
	}

	ObjectSavingContext savingCtx;
	Buffer data;
	EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
	EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
	auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, ComparePreciseNumericDouble)
{
	Random r;

	auto ptr = RefNew<TestObject>();
	for (uint32_t i = 0; i < 1000; ++i)
	{
		ptr->arrayDouble.pushBack(r.rangeDouble(-63356.0f, 63356.0f));
	}

	ObjectSavingContext savingCtx;
	Buffer data;
	EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
	EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
	auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareArrayCompound)
{
	auto ptr = RefNew<TestObject>();
    ptr->arrayCompound.pushBack(Vector3(1, 2, 3));
	ptr->arrayCompound.pushBack(Vector3(2, 3, 4));
	ptr->arrayCompound.pushBack(Vector3(3, 4, 5));
	ptr->arrayCompound.pushBack(Vector3(5, 6, 7));
	ptr->arrayCompound.pushBack(Vector3(8, 9, 103));

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
    EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, ComparePointerSerialization)
{
	auto ptr = RefNew<TestObject>();
    ptr->ptr = RefNew<TestObject>();
    ptr->ptr->simple = 123.0f;

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
	EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CompareInlineBufferSerialization)
{
	auto ptr = RefNew<TestObject>();
    ptr->buf = Buffer::CreateFromCopy(MainPool(), StringView("AlaMaKota"));

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
	EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, LoadesEmptyPromiseResourceIfNoFactory)
{
    auto id = ResourceID::Create();
    auto promise = ResourcePromise::CreateEmptyPromise(id, TestResource::GetStaticClass());

	auto ptr = RefNew<TestObject>();
    ptr->resRef = BaseReference(promise);

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
	EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;
    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr);
    EXPECT_EQ(nullptr, ptr->resRef.resolveResource());
	ptr2->Compare(ptr);
}

TEST(BinarySerialization, CreatesResourcePromiseViaFactory)
{
	auto id = ResourceID::Create();
	auto promise = ResourcePromise::CreateEmptyPromise(id, TestResource::GetStaticClass());

	auto ptr = RefNew<TestObject>();
	ptr->resRef = BaseReference(promise);

	ObjectSavingContext savingCtx;
    Buffer data;
    EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
	EXPECT_TRUE(data);

	ObjectLoadingContext loadingCtx;

    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr2);
    
    auto promiseId = ptr2->resRef.resolveId();
    EXPECT_EQ(id, promiseId);
}

TEST(BinarySerialization, AsyncBufferSaves)
{
    Buffer data;

    {
        auto ptr = RefNew<TestObject>();
        auto buf = Buffer::CreateFromCopy(MainPool(), StringView("Ala ma kota"));
        ptr->abuf.setup(buf);

        ObjectSavingContext savingCtx;
        EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, savingCtx, ptr, data));
        EXPECT_TRUE(data);
    }

	ObjectLoadingContext loadingCtx;

    auto ptr2 = IObject::LoadObject<TestObject>(SerializationFormat::RawBinary, data);
	ASSERT_NE(nullptr, ptr2);
    ASSERT_TRUE(ptr2->abuf);

    auto data2 = ptr2->abuf.load(NoTask(), MainPool());
    ASSERT_FALSE(data2.empty());

    auto txt = StringView(data2.view());
    EXPECT_TRUE(txt == "Ala ma kota");
}

TEST(BinarySerialization, Save100)
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

        ObjectSavingContext ctx;
        Buffer data;
        EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, ctx, layer, data));

        TRACE_INFO("Saving time 100: {}", timer);
        EXPECT_TRUE(data);
    }    
}

TEST(BinarySerialization, Save1000)
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

        Buffer data;
        ObjectSavingContext ctx;
        EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, ctx, layer, data));

		TRACE_INFO("Saving time 1000: {}", timer);
		EXPECT_TRUE(data);
	}
}

TEST(BinarySerialization, Save10K)
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

            Buffer data;
            ObjectSavingContext ctx;
            EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, ctx, layer, data));

            TRACE_WARNING("Saving time 10K: {}", timer);
            EXPECT_TRUE(data);
        }
        /*{
			TRACE_WARNING("Just finished");
            IPool::PrintPools(TRACE_STREAM_WARNING());
			Thread::Sleep(2000);
        }*/
	}
}

#if 0
TEST(BinarySerialization, Save100K)
{
	bm::Random r;
	TestLayerGenerationSettings settings;
	settings.maxChildrenPerEntity = 3;
	settings.maxComponentsPerEntity = 10;
	settings.maxEntityDepth = 2;
	settings.maxLinksPerEntity = 0;
	settings.numEntities = 100000;

	auto layer = GenerateTestLayer(r, settings);
	for (;;)
	{
		ScopeTimer timer;

        Buffer data;
        ObjectSavingContext ctx;
        EXPECT_TRUE(IObject::SaveObject(SerializationFormat::RawBinary, ctx, layer, data));
		TRACE_ERROR("Saving time 100K: {}", timer);

		//auto compressed = Buffer::CreateCompressed(MainPool(), CompressionType::Zlib, data);
		//TRACE_INFO("Objects compressed: {} -> {}", MemSize(data.size()), MemSize(compressed.size()));

		EXPECT_TRUE(data);
	}
}
#endif

//--

END_INFERNO_NAMESPACE_EX(test)
