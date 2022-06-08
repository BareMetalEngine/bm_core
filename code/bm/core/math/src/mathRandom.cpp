/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math #]
***/

#include "build.h"
#include "mathRandom.h"
#include "box.h"
#include "sphere.h"
#include "obb.h"

#include "bm/core/system/include/timing.h"

BEGIN_INFERNO_NAMESPACE()

//--

static const double INV_MAX_UINT32 = 1.0 / (double)std::numeric_limits<uint32_t>::max();
static const double INV_MAX_UINT64 = 1.0 / (double)std::numeric_limits<uint64_t>::max();

//--

RTTI_BEGIN_TYPE_STRUCT(Random);

RTTI_PROPERTY(m_state).scriptVisible().scriptReadOnly();

RTTI_SCRIPT_STATIC_FUNCTION(RectPoint);
RTTI_SCRIPT_STATIC_FUNCTION(CirclePoint);
RTTI_SCRIPT_STATIC_FUNCTION(BoxPointMinMax);
RTTI_SCRIPT_STATIC_FUNCTION(BoxPoint);
RTTI_SCRIPT_STATIC_FUNCTION(OOBPoint);
RTTI_SCRIPT_STATIC_FUNCTION(UnitSpherePoint);
RTTI_SCRIPT_STATIC_FUNCTION(SpherePoint);
RTTI_SCRIPT_STATIC_FUNCTION(SpherePointEx);
RTTI_SCRIPT_STATIC_FUNCTION(UnitSphereSurfacePoint);
RTTI_SCRIPT_STATIC_FUNCTION(SphereSurfacePoint);
RTTI_SCRIPT_STATIC_FUNCTION(SphereSurfacePointEx);
RTTI_SCRIPT_STATIC_FUNCTION(UnitHemiSphereSurfacePoint);
RTTI_SCRIPT_STATIC_FUNCTION(UnitSphereSurfacePointFast);
RTTI_SCRIPT_STATIC_FUNCTION(UnitHemiSphereSurfacePointWithNormal);
RTTI_SCRIPT_STATIC_FUNCTION(HemiSphereSurfacePointWithNormalEx);
RTTI_SCRIPT_STATIC_FUNCTION(UnitTrianglePoint);
RTTI_SCRIPT_STATIC_FUNCTION(TrianglePoint);

RTTI_SCRIPT_CLASS_FUNCTION_EX("Seed", seed);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextUint32", next);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextFloat", unit);
//RTTI_SCRIPT_CLASS_FUNCTION_EX("NextDouble", unitDP);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextVector2", unit2);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextVector3", unit3);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextVector4", unit4);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextRange", range);
RTTI_SCRIPT_CLASS_FUNCTION_EX("NextSelect", select);

RTTI_END_TYPE();

//--

static std::atomic<uint64_t> GRandomInitializer = NativeTimePoint::Now().rawValue() * 137;
static std::atomic<uint64_t> GRandomInitializer2 = NativeTimePoint::Now().rawValue() * 57;
static std::atomic<uint64_t> GRandomInitializer3 = NativeTimePoint::Now().rawValue() * 331;

Random::Random()
{
    reset();
}

void Random::reset()
{
    auto a = GRandomInitializer++;
    auto b = (a & 1) ? GRandomInitializer2++ : GRandomInitializer3++;

    CRC64 crc;
    crc << NativeTimePoint::Now().rawValue();
    crc << a;
    crc << b;

    m_state = crc.crc();
}

void Random::seed(uint64_t value)
{
    m_state = value;
}

float Random::unit()
{
    return (float)(next() * INV_MAX_UINT32);
}

double Random::unitDouble()
{
    return nextUint64() * INV_MAX_UINT64;
}

uint32_t Random::next()
{
    uint64_t z = (m_state + UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30))* UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27))* UINT64_C(0x94D049BB133111EB);
    m_state = z ^ (z >> 31);
    return (uint32_t)m_state;
}

uint64_t Random::nextUint64()
{
    uint64_t hi = next();
    uint64_t lo = next();
    return (hi << 32) | lo;
}

//--

Vector2 Random::RectPoint(const Vector2& rand, const Vector2& min, const Vector2& max)
{
    return min + rand * (max - min);
}

Vector2 Random::CirclePoint(const Vector2& rand, const Vector2& center, float radius)
{
    auto r = std::sqrt(rand.x);
    auto a = rand.x * TWOPI;
    return Vector2(r * std::cos(a), r * std::sin(a));
}

Vector3 Random::BoxPointMinMax(const Vector3& rand, const Vector3& min, const Vector3& max)
{
    return min + rand * (max - min);
}

Vector3 Random::BoxPoint(const Vector3& rand, const Box& box)
{
    return box.min + rand * box.extents();
}

Vector3 Random::OOBPoint(const Vector3& rand, const OBB& box)
{
    Vector3 ret = box.position();
    ret += box.edge1AndLength.xyz() * rand.x * box.edge1AndLength.w;
    ret += box.edge2AndLength.xyz() * rand.y * box.edge2AndLength.w;
    ret += box.edgeC() * rand.z * box.positionAndLength.w;
    return ret;
}

Vector3 Random::UnitSpherePoint(const Vector3& rand)
{
    auto theta = rand.x * 2.0f * PI;
    auto phi = std::acos(2.0f * rand.y - 1.0);
    auto r = std::cbrt(rand.z);
    auto sinTheta = std::sin(theta);
    auto cosTheta = std::cos(theta);
    auto sinPhi = std::sin(phi);
    auto cosPhi = std::cos(phi);
    return Vector3(r * sinPhi * cosTheta, r * sinPhi * sinTheta, r * cosPhi);
}

Vector3 Random::SpherePointEx(const Vector3& rand, const Vector3& center, float radius)
{
    return center + (UnitSpherePoint(rand) * radius);
}

Vector3 Random::SpherePoint(const Vector3& rand, const Sphere& sphere)
{
    return sphere.position() + (UnitSpherePoint(rand) * sphere.radius());
}

Vector3 Random::UnitSphereSurfacePoint(const Vector2& rand)
{
    auto theta = TWOPI * rand.x;
    auto phi = std::acos(1.0f - 2.0f * rand.y);
    return Vector3(std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi));
}

Vector3 Random::SphereSurfacePointEx(const Vector2& rand, const Vector3& center, float radius)
{
    return center + (UnitSphereSurfacePoint(rand) * radius);
}

Vector3 Random::SphereSurfacePoint(const Vector2& rand, const Sphere& sphere)
{
    return sphere.position() + (UnitSphereSurfacePoint(rand) * sphere.radius());
}

Vector3 Random::UnitHemiSphereSurfacePoint(Vector2 rand)
{
    auto azimuthal = TWOPI * rand.x;

    auto xyproj = std::sqrt(1 - rand.y * rand.y);
    return Vector3(xyproj * std::cos(azimuthal), xyproj * std::sin(azimuthal), rand.y);
}

Vector3 Random::UnitSphereSurfacePointFast(Vector2 rand)
{
    auto azimuthal = TWOPI * rand.x;

    // map [0,1) to [0,1) or [-1,0) with half probability
    auto y = (2.0f * rand.y);
    if (rand.y < 0.5f)
        y += -2.0f; // [1-2) -> [-1,0)

    auto xyproj = std::sqrt(1 - y * y); // cancels sign change
    return Vector3(xyproj * std::cos(azimuthal), xyproj * std::sin(azimuthal), y);
}

Vector3 Random::UnitHemiSphereSurfacePointWithNormal(const Vector2& rand, const Vector3& normal)
{
    auto spherePoint = UnitHemiSphereSurfacePoint(rand);
    if ((spherePoint | normal) < 0.0f)
        spherePoint = -spherePoint;
    return spherePoint;
}

Vector3 Random::HemiSphereSurfacePointWithNormalEx(const Vector2& rand, const Vector3& center, float radius, const Vector3& normal)
{
    return center + (UnitHemiSphereSurfacePointWithNormal(rand, normal) * radius);
}

Vector3 Random::UnitTrianglePoint(const Vector2& rand)
{
    auto su0 = std::sqrt(rand.x);
    auto su1 = rand.y * su0;
    return Vector3(1.0f - su0, su1, su0 - su1);
}

Vector3 Random::TrianglePoint(const Vector2& rand, const Vector3& a, const Vector3& b, const Vector3& c)
{
    auto pos = UnitTrianglePoint(rand);
    return (a * pos.x) + (b * pos.y) + (c * pos.z);
}

//--

#if 0
//--

#define MT_SIZE 624
#define MT_PERIOD 397
#define MT_DIFF MT_SIZE - MT_PERIOD
#define MT_MAGIC 0x9908b0df

#define M32(x) (0x80000000 & x) // 32nd MSB
#define L31(x) (0x7FFFFFFF & x) // 31 LSBs

#define UNROLL(expr) \
    y = M32(MT[i]) | L31(MT[i+1]); \
    MT[i] = MT[expr] ^ (y >> 1) ^ (((int(y) << 31) >> 31) & MT_MAGIC); \
    ++i;

void MTRandState::generate()
{
    int i = 0;
    uint32_t y;

    while (i < MT_DIFF)
    {
        UNROLL(i + MT_PERIOD);
        UNROLL(i + MT_PERIOD);
    }

    while (i < MT_SIZE - 1)
    {
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
        UNROLL(i - MT_DIFF);
    }

    {
        y = M32(MT[MT_SIZE - 1]) | L31(MT[0]);
        MT[MT_SIZE - 1] = MT[MT_PERIOD - 1] ^ (y >> 1) ^ (((int32_t(y) << 31) >> 31)& MT_MAGIC);
    }

    for (size_t i = 0; i < MT_SIZE; ++i)
    {
        y = MT[i];
        y ^= y >> 11;
        y ^= y << 7 & 0x9d2c5680;
        y ^= y << 15 & 0xefc60000;
        y ^= y >> 18;
        MT_TEMPERED[i] = y;
    }

    index = 0;
}


void MTRandState::seed(uint32_t seed)
{
    MT[0] = seed;
    for (uint32_t i = 1; i < MT_SIZE; ++i)
        MT[i] = 0x6c078965 * (MT[i - 1] ^ MT[i - 1] >> 30) + i;

    index = MT_SIZE; // generate on next get
}

uint32_t MTRandState::next()
{
    if (index == MT_SIZE)
        generate();

    return MT_TEMPERED[index++];
}

MTRandState::MTRandState(uint32_t val /*= 0*/)
{
    seed(val);
}

#endif

//--

END_INFERNO_NAMESPACE()
