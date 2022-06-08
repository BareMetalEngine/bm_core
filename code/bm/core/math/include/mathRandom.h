/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

// common random utilities
struct BM_CORE_MATH_API Random
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(Random);

public:
    Random();

    // convert 2 uniform random variables to a random 2D point in a rectangle with uniform distribution
    static Vector2 RectPoint(const Vector2& rand, const Vector2& min, const Vector2& max);

    // convert 2 uniform random variables to a random 2D point in a circle with uniform distribution, NOTE: circle boundary is excluded
    static Vector2 CirclePoint(const Vector2& rand, const Vector2& center, float radius);

    // convert 3 uniform random variables to a random 3D point inside a box
    static Vector3 BoxPointMinMax(const Vector3& rand, const Vector3& min, const Vector3& max);

    // convert 3 uniform random variables to a random 3D point inside a box
    static Vector3 BoxPoint(const Vector3& rand, const Box& box);

    // convert 3 uniform random variables to a random 3D point inside a box
    static Vector3 OOBPoint(const Vector3& rand, const OBB& box);

    // convert 3 uniform random variables to a random 3D point inside an sphere
    static Vector3 UnitSpherePoint(const Vector3& rand);

    // convert 3 uniform random variables to a random 3D point inside a sphere
    static Vector3 SpherePoint(const Vector3& rand, const Sphere& sphere);

    // convert 3 uniform random variables to a random 3D point inside a sphere
    static Vector3 SpherePointEx(const Vector3& rand, const Vector3& center, float radius);

    // convert 2 (TWO) uniform random variables to a random 3D point ON A SURFACE of the unit sphere
    static Vector3 UnitSphereSurfacePoint(const Vector2& rand);

    // convert 3 uniform random variables to a random 3D point inside a sphere
    static Vector3 SphereSurfacePoint(const Vector2& rand, const Sphere& sphere);

    // convert 2 (TWO) uniform random variables to a random 3D point ON A SURFACE of the sphere
    static Vector3 SphereSurfacePointEx(const Vector2& rand, const Vector3& center, float radius);

    // convert 2 (TWO) uniform random variables to a random 3D point on a unit hemi-sphere at 0,0,0 and with N=0,0,1 (standard tangent-space hemisphere)
    static Vector3 UnitHemiSphereSurfacePoint(Vector2 rand);

    // convert 2 (TWO)
    static Vector3 UnitSphereSurfacePointFast(Vector2 rand);

    // convert 2 (TWO) uniform random variables to a random 3D point on a unit hemi-sphere at 0,0,0 with specified N
    static Vector3 UnitHemiSphereSurfacePointWithNormal(const Vector2& rand, const Vector3& N);

    // convert 2 (TWO) uniform random variables to a random 3D point on a hemi-sphere facing given direction
    static Vector3 HemiSphereSurfacePointWithNormalEx(const Vector2& rand, const Vector3& center, float radius, const Vector3& normal);

    // convert 2 (TWO) uniform random variables to a random point inside a unit triangle (basically we return the barycentric coordinates)
    static Vector3 UnitTrianglePoint(const Vector2& rand);

    // convert 2 (TWO) uniform random variables to a random point inside a triangle
    static Vector3 TrianglePoint(const Vector2& rand, const Vector3& a, const Vector3& b, const Vector3& c);

    //--

    // reset to random seed
    void reset();

    // set custom seed value
    void seed(uint64_t value);

    // get next random value, full 32bit of randomness
    uint32_t next();

	// get next random value, full 64bit of randomness
	uint64_t nextUint64();

    // single random scalar [0,1) range
    float unit();

    //--

    // 2D vector [0,1)
    INLINE Vector2 unit2();

    // 3D vector [0,1)
    INLINE Vector3 unit3();

    // 3D vector [0,1)
    INLINE Vector4 unit4();

    // [0, max-1], returns 0 if max is 0
    INLINE uint32_t select(uint32_t max);

    //---

    // [0,1> - double precission
    double unitDouble();

    // [min,max> - single precission
    INLINE float range(float min, float max);

    // [min,max>
    INLINE double rangeDouble(double min, double max);

private:
    uint64_t m_state = 0;
};

//---

END_INFERNO_NAMESPACE()
