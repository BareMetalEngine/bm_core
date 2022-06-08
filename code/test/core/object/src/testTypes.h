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
#include "bm/core/parser/include/textToken.h"

BEGIN_INFERNO_NAMESPACE_EX(test)

//--

struct TestTransform
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(TestTransform);

public:
    Vector3 pos;
    Angles rot;
    Vector3 scale;
};

class TestResource : public IResource
{
    RTTI_DECLARE_OBJECT_CLASS(TestResource, IResource);

public:
    ResourceID m_id;

    TestResource(ResourceID id = ResourceID())
        : m_id(id)
    {}
};

class TestObject : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(TestObject, IObject);

public:
    bool trivial = 0;
    float simple = 0.0f;
    StringBuf txt;
    Vector3 compound;
    Box compoundEx;
    RefPtr<TestObject> ptr;
    RefWeakPtr<TestObject> weakPtr;
    ResourceRef<TestResource> resRef;
    Array<float> arraySimple;
    Array<Vector3> arrayCompound;
    Array<Box> arrayCompoundEx;
    Array<RefPtr<TestObject>> arrayPtrs;
    Array<float> arrayFloat;
    Array<double> arrayDouble;
    Buffer buf;
    AsyncFileBuffer abuf;

    void Compare(TestObject* base);
};

class TestComponent : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(TestComponent, IObject);

public:
    StringID name;
    TestTransform transform;

    void Compare(TestComponent* base);
};

class TestResourceComponent : public TestComponent
{
    RTTI_DECLARE_OBJECT_CLASS(TestResourceComponent, TestComponent);

public:
    ResourceRef<IResource> mesh;

    void Compare(TestResourceComponent* base);
};

class TestNonResourceComponent : public TestComponent
{
	RTTI_DECLARE_OBJECT_CLASS(TestNonResourceComponent, TestComponent);

public:
    Color color = Color::WHITE;
    float brightness = 1.0f;
    float radius = 10.0f;
    StringBuf text;

    void Compare(TestNonResourceComponent* base);
};

class TestLink : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(TestLink, IObject);

public:
    RefPtr<TestComponent> src;
    RefPtr<TestComponent> dest;
};

class TestEntity : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(TestEntity, IObject);

public:
    StringID name;
    TestTransform transform;
    Array<RefPtr<TestEntity>> children;
    Array<RefPtr<TestComponent>> components;

    void Compare(TestEntity* base);
};

class TestLayer : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(TestLayer, IObject);

public:
	Array<RefPtr<TestEntity>> entities;

    void Compare(TestLayer* base);
};

//--

struct TestLayerGenerationSettings
{
    Array<ResourcePromisePtr> resources;

    struct Name
    {
        StringBuf txt;
        uint32_t count = 0;
    };

    Array<Name> names;

    uint32_t numEntities = 0;

    uint32_t maxComponentsPerEntity = 0;
    uint32_t maxChildrenPerEntity = 0;
    uint32_t maxLinksPerEntity = 0;
    uint32_t maxEntityDepth = 2;

    StringID nextName(bm::Random& r);
    TestLayerGenerationSettings(uint32_t numResources = 10, uint32_t numCoreames = 128);
};

void GenerateTestTransform(bm::Random& r, TestTransform& t);

RefPtr<TestComponent> GenerateTestComponent(bm::Random& r, TestLayerGenerationSettings& setup);
RefPtr<TestEntity> GenerateTestEntity(bm::Random& r, TestLayerGenerationSettings& setup, uint32_t depth);
RefPtr<TestLayer> GenerateTestLayer(bm::Random& r, TestLayerGenerationSettings& setup);

enum class TestEnum : uint8_t
{
    First,
    Second,
    Third,
};

enum class TestFlagBit : uint8_t
{
	First,
	Second,
	Third,
};

typedef BitFlags<TestFlagBit> TestFlags;

//--

class TestErrorReporter : public ITextErrorReporter
{
public:
    virtual void reportError(const TextTokenLocation& loc, StringView message);
    virtual void reportWarning(const TextTokenLocation& loc, StringView message);

    StringBuf lastErrorText;
    TextTokenLocation lastErrorLocation;

	StringBuf lastWarningText;
	TextTokenLocation lastWarningLocation;

    uint32_t numErrors = 0;
    uint32_t numWarnings = 0;
};

//--

END_INFERNO_NAMESPACE_EX(test)
