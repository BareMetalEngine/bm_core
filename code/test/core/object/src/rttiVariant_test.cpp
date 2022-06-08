/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringBuf.h"
#include "bm/core/object/include/rttiVariant.h"

DECLARE_TEST_FILE(RttiVariant);

BEGIN_INFERNO_NAMESPACE_EX(test)

TEST(VariantTest, CreateEmpty)
{
    Variant a;
    ASSERT_TRUE(a.empty());
    ASSERT_FALSE(a.type());
    ASSERT_EQ(nullptr, a.data());
}

TEST(VariantTest, EmptyVariantDoesNotPrint)
{
    Variant a;
    ASSERT_TRUE(a.empty());
    ASSERT_FALSE(a.type());

    StringBuilder txt;
    a.print(txt);

    ASSERT_TRUE(txt.empty());
}

TEST(VariantTest, EmptyVariantDoesDebugPrintEmpty)
{
    Variant a;
    ASSERT_TRUE(a.empty());
    ASSERT_FALSE(a.type());

    StringBuilder txt;
    a.print(txt);

    ASSERT_STREQ("", txt.c_str());
}

TEST(VariantTest, CreateInt8)
{
    Variant a = Variant((char)42);
    ASSERT_TRUE(a.type() == GetTypeObject<char>());

    auto ptr  = *(const char*)a.data();
    ASSERT_EQ(ptr, 42);
}

TEST(VariantTest, CreateInt16)
{
    Variant a = Variant((short)42);
    ASSERT_TRUE(a.type() == GetTypeObject<short>());

    auto ptr = *(const short*)a.data();
    ASSERT_EQ(ptr, 42);
}


TEST(VariantTest, CreateInt32)
{
    Variant a = Variant((int)42);
    ASSERT_TRUE(a.type() == GetTypeObject<int>());

    auto ptr = *(const int*)a.data();
    ASSERT_EQ(ptr, 42);
}

TEST(VariantTest, CreateInt64)
{
    Variant a = Variant((int64_t)42);
    ASSERT_TRUE(a.type() == GetTypeObject<int64_t>());

    auto ptr = *(const int64_t*)a.data();
    ASSERT_EQ(ptr, 42);
}

TEST(VariantTest, CreateUint8)
{
    Variant a = Variant((uint8_t)42);
    ASSERT_TRUE(a.type() == GetTypeObject<uint8_t>());

    auto ptr = *(const uint8_t*)a.data();
    ASSERT_EQ(ptr, 42);
}

TEST(VariantTest, CreateUint16)
{
    Variant a = Variant((uint16_t)42);
    ASSERT_TRUE(a.type() == GetTypeObject<uint16_t>());

    auto ptr = *(const uint16_t*)a.data();
    ASSERT_EQ(ptr, 42);
}


TEST(VariantTest, CreateUint32)
{
    Variant a = Variant((uint32_t)42);
    ASSERT_TRUE(a.type() == GetTypeObject<uint32_t>());

    auto ptr = *(const uint32_t*)a.data();
    ASSERT_EQ(ptr, 42);
}

TEST(VariantTest, CreateUint64)
{
    Variant a = Variant((uint64_t)42);
    ASSERT_TRUE(a.type() == GetTypeObject<uint64_t>());

    auto ptr = *(const uint64_t*)a.data();
    ASSERT_EQ(ptr, 42);
}

TEST(VariantTest, CreateBool)
{
    Variant a = Variant(true);
    ASSERT_TRUE(a.type() == GetTypeObject<bool>());

    auto ptr = *(const bool*)a.data();
    ASSERT_EQ(ptr, true);
}

TEST(VariantTest, CreateFloat)
{
    Variant a = Variant(3.141592f);
    ASSERT_TRUE(a.type() == GetTypeObject<float>());

    auto ptr = *(const float*)a.data();
    ASSERT_FLOAT_EQ(3.141592f, ptr);
}

TEST(VariantTest, CreateDouble)
{
    Variant a = Variant(3.141592);
    ASSERT_TRUE(a.type() == GetTypeObject<double>());

    auto ptr = *(const double*)a.data();
    ASSERT_FLOAT_EQ(3.141592f, ptr);
}

TEST(VariantTest, CreateString)
{
    Variant a = Variant(StringBuf("Ala ma kota"));
    ASSERT_TRUE(a.type() == GetTypeObject<StringBuf>());

    const auto& str = *(const StringBuf*)a.data();
    ASSERT_STREQ("Ala ma kota", str.c_str());
}

TEST(VariantTest, CreateStringID)
{
    Variant a = Variant("TestIdent"_id);
    ASSERT_TRUE(a.type() == GetTypeObject<StringID>());

    const auto& str = *(const StringID*)a.data();
    ASSERT_STREQ("TestIdent", str.c_str());
}

/*TEST(VariantTest, CreateVector3)
{
    Variant a = CreateVariant<Vector3>(Vector3(1, 2, 3));
    ASSERT_TRUE(a.type() == GetTypeObject<Vector3>());

    const auto& str = *(const Vector3*)a.data();
    EXPECT_FLOAT_EQ(1.0f, str.x);
    EXPECT_FLOAT_EQ(2.0f, str.y);
    EXPECT_FLOAT_EQ(3.0f, str.z);
}*/

TEST(VariantTest, CreateDynamicArray)
{
    Array<int> table;
    table.pushBack(1);
    table.pushBack(2);
    table.pushBack(3);
    table.pushBack(4);

    Variant a = Variant(table);
    ASSERT_TRUE(a.type() == GetTypeObject<Array<int>>());

    const auto& str = *(const Array<int>*)a.data();
    ASSERT_EQ(4, str.size());
    EXPECT_EQ(1, str[0]);
    EXPECT_EQ(2, str[1]);
    EXPECT_EQ(3, str[2]);
    EXPECT_EQ(4, str[3]);
}

TEST(VariantTest, CreateStaticArray)
{
    int table[4];
    table[0] = 1;
    table[1] = 2;
    table[2] = 3;
    table[3] = 4;

    Variant a = Variant(table);
    ASSERT_TRUE(a.type().isArray());

    const auto* str = (const int*)a.data();
    EXPECT_EQ(1, str[0]);
    EXPECT_EQ(2, str[1]);
    EXPECT_EQ(3, str[2]);
    EXPECT_EQ(4, str[3]);
}

class VariantTestClass : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(VariantTestClass, IObject);

public:
    VariantTestClass()
    {}

    int data = 0;
    bool* destroyFlag = nullptr;

    virtual ~VariantTestClass()
    {
        if (destroyFlag)
            *destroyFlag = true;
    }
};

RTTI_BEGIN_TYPE_CLASS(VariantTestClass);
RTTI_END_TYPE();

TEST(VariantTest, CreateVariantSharedPtr)
{
    auto ptr = RefNew<VariantTestClass>();
    Variant a = Variant(ptr);
    ASSERT_TRUE(a.type() == GetTypeObject<RefPtr<VariantTestClass>>());
}

TEST(VariantTest, SharedPtrVariantPointsToTheSameObject)
{
    auto ptr = RefNew<VariantTestClass>();
    Variant a = Variant(ptr);
    ASSERT_TRUE(a.type() == GetTypeObject<RefPtr<VariantTestClass>>());

    const auto& str = *(const RefPtr<VariantTestClass>*)a.data();
    ASSERT_EQ(str.get(), ptr.get());
}

TEST(VariantTest, SharedPtrVariantKeepsObjectAlive)
{
    bool destroyFlag = false;

    auto ptr = RefNew<VariantTestClass>();
    ptr->destroyFlag = &destroyFlag;

    Variant a = Variant(ptr);
    ASSERT_TRUE(a.type() == GetTypeObject<RefPtr<VariantTestClass>>());
    ASSERT_FALSE(destroyFlag);

    ptr.reset();

    ASSERT_FALSE(destroyFlag);

    a.reset();

    ASSERT_TRUE(destroyFlag); // now object should be destroyed
}

/*
TEST(Variant, CreateStruct)
{
    Matrix init;
    Variant a = CreateVariant<Matrix>(init);
    ASSERT_FALSE(a.empty());

    std::string str = a;
    std::string pat = "variant(Matrix) = { \"class\" : \"Matrix\", \"id\" : \"1\", \"x\" : { \"class\" : \"Vector4\", \"id\" : \"1\", \"x\" : \"1.000000\"}, \"y\" : { \"class\" : \"Vector4\", \"id\" : \"2\", \"y\" : \"1.000000\"}, \"z\" : { \"class\" : \"Vector4\", \"id\" : \"3\", \"z\" : \"1.000000\"}, \"w\" : { \"class\" : \"Vector4\", \"id\" : \"4\", \"w\" : \"1.000000\"}}";
    ASSERT_EQ(pat, str);
}*/

END_INFERNO_NAMESPACE_EX(test)
