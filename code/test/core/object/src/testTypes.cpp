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

RTTI_BEGIN_TYPE_STRUCT(TestTransform);
RTTI_PROPERTY(pos);
RTTI_PROPERTY(rot);
RTTI_PROPERTY(scale);
RTTI_END_TYPE();

RTTI_BEGIN_TYPE_CLASS(TestResource);
RTTI_END_TYPE();

void TestObject::Compare(TestObject* base)
{
    EXPECT_EQ(base->trivial, trivial);
    EXPECT_EQ(base->simple, simple);
    EXPECT_EQ(base->txt, txt);
    EXPECT_EQ(base->compound.x, compound.x);
    EXPECT_EQ(base->compound.y, compound.y);
    EXPECT_EQ(base->compound.z, compound.z);
    EXPECT_EQ(base->compoundEx.min.x, compoundEx.min.x);
    EXPECT_EQ(base->compoundEx.min.y, compoundEx.min.y);
    EXPECT_EQ(base->compoundEx.min.z, compoundEx.min.z);
	EXPECT_EQ(base->compoundEx.max.x, compoundEx.max.x);
	EXPECT_EQ(base->compoundEx.max.y, compoundEx.max.y);
	EXPECT_EQ(base->compoundEx.max.z, compoundEx.max.z);
    EXPECT_EQ(base->arraySimple, arraySimple);
    EXPECT_EQ(base->arrayCompound, arrayCompound);
    EXPECT_EQ(base->arrayCompoundEx, arrayCompoundEx);

    ASSERT_EQ(base->arrayFloat.size(), arrayFloat.size());
    for (uint32_t i = 0; i < arrayFloat.size(); ++i)
    {
        EXPECT_FLOAT_EQ(base->arrayFloat[i], arrayFloat[i]);
    }

	ASSERT_EQ(base->arrayDouble.size(), arrayDouble.size());
	for (uint32_t i = 0; i < arrayDouble.size(); ++i)
	{
		EXPECT_FLOAT_EQ(base->arrayDouble[i], arrayDouble[i]);
	}

    EXPECT_EQ(!!base->ptr, !!ptr);
    if (ptr && base->ptr)
        ptr->Compare(base->ptr);
    EXPECT_EQ(!!base->buf, !!buf);
    if (buf && base->buf)
    {
        EXPECT_EQ(base->buf.size(), buf.size());
        if (base->buf.size() == buf.size())
        {
            EXPECT_EQ(0, base->buf.view().compareMemory(buf.view()));
        }
    }
    EXPECT_EQ(!!base->resRef, !!resRef);
    EXPECT_EQ(base->resRef.resolveId(), resRef.resolveId());

    //EXPECT_EQ(base->arrayCompoundEx, arrayCompoundEx);
}

RTTI_BEGIN_TYPE_CLASS(TestObject);
RTTI_PROPERTY(trivial);
RTTI_PROPERTY(simple);
RTTI_PROPERTY(txt);
RTTI_PROPERTY(compound);
RTTI_PROPERTY(compoundEx);
RTTI_PROPERTY(ptr);
RTTI_PROPERTY(weakPtr);
RTTI_PROPERTY(arraySimple);
RTTI_PROPERTY(arrayCompound);
RTTI_PROPERTY(arrayCompoundEx);
RTTI_PROPERTY(arrayPtrs);
RTTI_PROPERTY(arrayFloat);
RTTI_PROPERTY(arrayDouble);
RTTI_PROPERTY(buf);
RTTI_PROPERTY(abuf);
RTTI_PROPERTY(resRef);
RTTI_END_TYPE();

RTTI_BEGIN_TYPE_CLASS(TestComponent);
RTTI_PROPERTY(name);
RTTI_PROPERTY(transform);
RTTI_END_TYPE();

void TestComponent::Compare(TestComponent* base)
{
    EXPECT_EQ(base->name, name);
    EXPECT_FLOAT_EQ(base->transform.pos.x, transform.pos.x);
    EXPECT_FLOAT_EQ(base->transform.pos.y, transform.pos.y);
    EXPECT_FLOAT_EQ(base->transform.pos.z, transform.pos.z);
	EXPECT_FLOAT_EQ(base->transform.rot.pitch, transform.rot.pitch);
	EXPECT_FLOAT_EQ(base->transform.rot.yaw, transform.rot.yaw);
	EXPECT_FLOAT_EQ(base->transform.rot.roll, transform.rot.roll);
	EXPECT_FLOAT_EQ(base->transform.scale.x, transform.scale.x);
	EXPECT_FLOAT_EQ(base->transform.scale.y, transform.scale.y);
	EXPECT_FLOAT_EQ(base->transform.scale.z, transform.scale.z);

	EXPECT_EQ((uint32_t&)base->transform.pos.x, (uint32_t&)transform.pos.x);
	EXPECT_EQ((uint32_t&)base->transform.pos.y, (uint32_t&)transform.pos.y);
	EXPECT_EQ((uint32_t&)base->transform.pos.z, (uint32_t&)transform.pos.z);
	EXPECT_EQ((uint32_t&)base->transform.rot.pitch, (uint32_t&)transform.rot.pitch);
	EXPECT_EQ((uint32_t&)base->transform.rot.yaw, (uint32_t&)transform.rot.yaw);
	EXPECT_EQ((uint32_t&)base->transform.rot.roll, (uint32_t&)transform.rot.roll);
	EXPECT_EQ((uint32_t&)base->transform.scale.x, (uint32_t&)transform.scale.x);
	EXPECT_EQ((uint32_t&)base->transform.scale.y, (uint32_t&)transform.scale.y);
	EXPECT_EQ((uint32_t&)base->transform.scale.z, (uint32_t&)transform.scale.z);
}

RTTI_BEGIN_TYPE_CLASS(TestResourceComponent);
RTTI_PROPERTY(mesh);
RTTI_END_TYPE();

void TestResourceComponent::Compare(TestResourceComponent* base)
{
	TBaseClass::Compare(base);
	EXPECT_EQ(mesh.resolveId(), base->mesh.resolveId());
}

RTTI_BEGIN_TYPE_CLASS(TestNonResourceComponent);
RTTI_PROPERTY(color);
RTTI_PROPERTY(brightness);
RTTI_PROPERTY(radius);
RTTI_PROPERTY(text);
RTTI_END_TYPE();

void TestNonResourceComponent::Compare(TestNonResourceComponent* base)
{
    TBaseClass::Compare(base);
    EXPECT_EQ(color.r, base->color.r);
    EXPECT_EQ(color.g, base->color.g);
    EXPECT_EQ(color.b, base->color.b);
    EXPECT_EQ(color.a, base->color.a);
    EXPECT_EQ(brightness, base->brightness);
    EXPECT_EQ(radius, base->radius);
    EXPECT_EQ(text, base->text);
}


RTTI_BEGIN_TYPE_CLASS(TestLink);
RTTI_PROPERTY(src);
RTTI_PROPERTY(dest);
RTTI_END_TYPE();

RTTI_BEGIN_TYPE_CLASS(TestEntity);
RTTI_PROPERTY(name);
RTTI_PROPERTY(transform);
RTTI_PROPERTY(children);
RTTI_PROPERTY(components);
RTTI_END_TYPE();

void TestEntity::Compare(TestEntity* base)
{
	EXPECT_EQ(base->name, name);
	EXPECT_FLOAT_EQ(base->transform.pos.x, transform.pos.x);
	EXPECT_FLOAT_EQ(base->transform.pos.y, transform.pos.y);
	EXPECT_FLOAT_EQ(base->transform.pos.z, transform.pos.z);
	EXPECT_FLOAT_EQ(base->transform.rot.pitch, transform.rot.pitch);
	EXPECT_FLOAT_EQ(base->transform.rot.yaw, transform.rot.yaw);
	EXPECT_FLOAT_EQ(base->transform.rot.roll, transform.rot.roll);
	EXPECT_FLOAT_EQ(base->transform.scale.x, transform.scale.x);
	EXPECT_FLOAT_EQ(base->transform.scale.y, transform.scale.y);
	EXPECT_FLOAT_EQ(base->transform.scale.z, transform.scale.z);

    ASSERT_EQ(base->children.size(), base->children.size());
    for (uint32_t i = 0; i < base->children.size(); ++i)
    {
        ASSERT_NE(nullptr, children[i]);
        children[i]->Compare(base->children[i]);
    }

	ASSERT_EQ(base->components.size(), base->components.size());
	for (uint32_t i = 0; i < base->components.size(); ++i)
	{
		ASSERT_NE(nullptr, components[i]);
        ASSERT_EQ(base->components[i]->cls(), components[i]->cls());

        if (components[i]->is<TestResourceComponent>())
            ((TestResourceComponent*)components[i].get())->Compare((TestResourceComponent*)(base->components[i].get()));
        else if (components[i]->is<TestNonResourceComponent>())
            ((TestNonResourceComponent*)components[i].get())->Compare((TestNonResourceComponent*)(base->components[i].get()));
	}
}

RTTI_BEGIN_TYPE_CLASS(TestLayer);
RTTI_PROPERTY(entities);
RTTI_END_TYPE();

void TestLayer::Compare(TestLayer* base)
{
	ASSERT_EQ(base->entities.size(), base->entities.size());
	for (uint32_t i = 0; i < base->entities.size(); ++i)
	{
		ASSERT_NE(nullptr, entities[i]);
        entities[i]->Compare(base->entities[i]);
	}
}

//--

StringID TestLayerGenerationSettings::nextName(bm::Random& r)
{
    auto index = r.select(names.size());

    auto& name = names[index];
    auto ret = StringID(TempString("{}_{}", name.txt, name.count));
    name.count += 1;
    return ret;
}

TestLayerGenerationSettings::TestLayerGenerationSettings(uint32_t numResources, uint32_t numCoreames)
{
    bm::Random r;
    for (uint32_t i = 0; i < numResources; ++i)
    {
        GUID g(r.next(), r.next(), r.next(), r.next());
        auto proxy = ResourcePromise::CreateEmptyPromise(g, TestResource::GetStaticClass());
        resources.pushBack(proxy);
    }

    for (uint32_t i = 0; i < numCoreames; ++i)
    {
        static const char* chars = "abcdefghijklmnopqrstuvwxyz";
        static const auto charsLen = strlen(chars);

        StringBuilder txt;

        for (uint32_t j = 0; j < 5; ++j)
        {
            auto ch = chars[r.select(charsLen)];
            txt.appendch(ch);
        }

        Name name;
        name.txt = StringBuf(txt);
        name.count = 1;
        names.pushBack(name);
    }
}

void GenerateTestTransform(bm::Random& r, TestTransform& t)
{
    if (r.next() & 1)
    {
        if (r.next() & 1) t.pos.x = r.range(-100.0f, 100.0f);
        if (r.next() & 1) t.pos.y = r.range(-100.0f, 100.0f);
        if (r.next() & 1) t.pos.z = r.range(-100.0f, 100.0f);
    }

    if (r.next() & 1)
    {
        if (r.next() & 1) t.rot.pitch = r.range(-180.0f, 180.0f);
        if (r.next() & 1) t.rot.yaw = r.range(-180.0f, 180.0f);
        if (r.next() & 1) t.rot.roll = r.range(-180.0f, 180.0f);
    }

    if (r.next() & 1) 
    {
		if (r.next() & 1) t.scale.x = r.range(0.1f, 2.0f);
        else if (r.next() & 1) t.scale.y = r.range(0.1f, 2.0f);
        else if (r.next() & 1) t.scale.z = r.range(0.1f, 2.0f);
	}
}

RefPtr<TestComponent> GenerateTestComponent(bm::Random& r, TestLayerGenerationSettings& setup)
{
    if (r.next() & 1)
    {
        auto ret = RefNew<TestResourceComponent>();

        auto resourceIndex = r.next() % setup.resources.size();
		ret->name = setup.nextName(r);
		GenerateTestTransform(r, ret->transform);
        ret->mesh = ResourceRef<IResource>(BaseReference(setup.resources[resourceIndex]));

        return ret;
    }
    else
    {
        auto ret = RefNew<TestNonResourceComponent>();
		ret->name = setup.nextName(r);
		GenerateTestTransform(r, ret->transform);

        if (r.next() & 1)
        {
            ret->color.r = r.next();
            ret->color.g = r.next();
            ret->color.b = r.next();
            ret->color.a = r.next();
        }
        if (r.next() & 1)
        {
            ret->radius = r.range(1.0f, 20.0f);
        }
		if (r.next() & 1)
		{
			ret->brightness = r.range(1.0f, 20.0f);
		}
        return ret;
    }
}

RefPtr<TestEntity> GenerateTestEntity(bm::Random& r, TestLayerGenerationSettings& setup, uint32_t depth)
{
    RefPtr<TestEntity> ret = RefNew<TestEntity>();

	ret->name = setup.nextName(r);
	GenerateTestTransform(r, ret->transform);

    {
        uint32_t numComponents = r.select(setup.maxComponentsPerEntity);
        for (uint32_t i = 0; i < numComponents; ++i)
        {
            auto comp = GenerateTestComponent(r, setup);
            ret->components.pushBack(comp);
            comp->parent(ret);
        }
    }

    if (depth < setup.maxEntityDepth)
    {
		uint32_t numEntities = r.select(setup.maxChildrenPerEntity);
		for (uint32_t i = 0; i < numEntities; ++i)
		{
			auto ent = GenerateTestEntity(r, setup, depth+1);
			ret->children.pushBack(ent);
            ent->parent(ret);
		}
    }

    // TODO: links

    return ret;

}

RefPtr<TestLayer> GenerateTestLayer(bm::Random& r, TestLayerGenerationSettings& setup)
{
    auto ret = RefNew<TestLayer>();

    for (uint32_t i = 0; i < setup.numEntities; ++i)
    {
		auto ent = GenerateTestEntity(r, setup, 1);
		ret->entities.pushBack(ent);
		ent->parent(ret);
    }

    return ret;
}

RTTI_BEGIN_TYPE_ENUM(TestEnum);
RTTI_ENUM_OPTION(First);
RTTI_ENUM_OPTION(Second);
RTTI_ENUM_OPTION(Third)
RTTI_END_TYPE();

RTTI_BEGIN_TYPE_BITFIELD(TestFlags);
RTTI_BITFIELD_OPTION(First);
RTTI_BITFIELD_OPTION(Second);
RTTI_BITFIELD_OPTION(Third)
RTTI_END_TYPE();

//--

void TestErrorReporter::reportError(const TextTokenLocation& loc, StringView message)
{
    lastErrorText = StringBuf(message);
    lastErrorLocation = loc;
    numErrors += 1;
}

void TestErrorReporter::reportWarning(const TextTokenLocation& loc, StringView message)
{
	lastWarningText = StringBuf(message);
	lastWarningLocation = loc;
	numWarnings += 1;
}

//--

END_INFERNO_NAMESPACE_EX(test)
