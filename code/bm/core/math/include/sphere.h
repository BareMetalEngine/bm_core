/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\sphere #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// sphere
class BM_CORE_MATH_API Sphere
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(Sphere);

public:
    //--

    Vector4 positionAndRadius = Vector4::ZERO(); //< position (x,y,z), radius (w), TODO: consider storing R*R

    //--


    INLINE Sphere() {};
    INLINE Sphere(const Sphere& other) = default;
    INLINE Sphere(Sphere&& other) = default;
    INLINE Sphere& operator=(const Sphere& other) = default;
    INLINE Sphere& operator=(Sphere&& other) = default;

    // create Sphere at given position and with given radius
    Sphere(const Vector3& pos, float radius);

    //--

    // get radius of the sphere
    INLINE float radius() const { return positionAndRadius.w; }

    // get position of the sphere's center
    INLINE const Vector3& position() const { return *(const Vector3*)&positionAndRadius; }

    // compute volume of the sphere
    float volume() const;

    // compute bounding box of the sphere
    Box bounds() const;

    //--

    // check if this sphere here hull contains a given point
    bool contains(const Vector3& point) const;

    // intersect this convex shape with ray, returns distance to point of entry
    bool intersect(const Vector3& origin, const Vector3& direction, float maxLength = VERY_LARGE_FLOAT, float* outEnterDistFromOrigin = nullptr, Vector3* outEntryPoint = nullptr, Vector3* outEntryNormal = nullptr) const;

    //--

private:
    INLINE Vector3 _getPosition() const { return *(const Vector3*)&positionAndRadius; }
    INLINE bool _intersect_NoPoint(const Vector3& origin, const Vector3& direction, float maxLength) const { return intersect(origin, direction, maxLength); }
    INLINE bool _intersect_WithPoint(const Vector3& origin, const Vector3& direction, float maxLength, float& outEnterDistFromOrigin, Vector3& outEntryPoint, Vector3& outEntryNormal) const { return intersect(origin, direction, maxLength, &outEnterDistFromOrigin, &outEntryPoint, &outEntryNormal); }
};

//---

END_INFERNO_NAMESPACE()
