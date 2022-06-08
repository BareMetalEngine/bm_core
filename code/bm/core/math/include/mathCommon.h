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

/// Plane sides - a special bit mask
enum PlaneSide
{
    SIDE_OnPlane = 0,
    SIDE_Positive = 1,
    SIDE_Negative = 2,
    SIDE_Split = 3,
};

//--

class Vector2;
class Vector3;
class Vector4;
class Matrix;
class Matrix33;
class Box;
class Plane;
class Quat;
class Transform;
class Angles;
class XForm2D;
class Color;
class Point;
class Rect;
class OBB;
class Sphere;
class Convex;
class TriMesh;
class Cylinder;
class Capsule;
class ExactPosition;
class EulerTransform;

//--

// normalize angel to -360 to 360 range
INLINE float AngleNormalize(float angle) { return angle - (std::trunc(angle / 360.0f) * 360.0f); }

// get distance between two angles, it's always in range from -180 to 180 deg
extern BM_CORE_MATH_API float AngleDistance(float srcAngle, float srcTarget);

// move given amount of degrees from starting angle towards the target angle
extern BM_CORE_MATH_API float AngleReach(float srcCurrent, float srcTarget, float move);

//--

//! Choose a plane that is the most perpendicular to given directions
extern BM_CORE_MATH_API bool GetMostPerpendicularPlane(const Vector3 &forward, const Vector3 &axis, const Vector3 &point, Plane &outPlane);

//! Calculate intersection of ray with plane
extern BM_CORE_MATH_API bool CalcPlaneRayIntersection(const Vector3& planeNormal, float planeDistance, const Vector3& rayOrigin, const Vector3& rayDir, float rayLength = VERY_LARGE_FLOAT, float* outDistance = nullptr, Vector3* outPosition = nullptr);

//! Calculate intersection of ray with plane
extern BM_CORE_MATH_API bool CalcPlaneRayIntersection(const Vector3& planeNormal, const Vector3& planePoint, const Vector3& rayOrigin, const Vector3& rayDir, float rayLength = VERY_LARGE_FLOAT, float* outDistance = nullptr, Vector3* outPosition = nullptr);

//! Calculate distance to an edge given by two endpoints, can also return closest point
extern BM_CORE_MATH_API float CalcDistanceToEdge(const Vector3& point, const Vector3& a, const Vector3 &b, Vector3* outClosestPoint);

//! Calculate two perpendicular vectors (UV)
extern BM_CORE_MATH_API void CalcPerpendicularVectors(const Vector3& dir, Vector3& outU, Vector3& outV);

//! Calculate normal of triangle from given vertices, if triangle is degenerated it returns zero vector
extern BM_CORE_MATH_API Vector3 TriangleNormal(const Vector3 &a, const Vector3 &b, const Vector3 &c);

//! Calculate area of the triangle
extern BM_CORE_MATH_API float TriangleArea(const Vector3& a, const Vector3& b, const Vector3& c);

//! Calculate normal if the given points create a triangle
extern BM_CORE_MATH_API bool SafeTriangleNormal(const Vector3 &a, const Vector3 &b, const Vector3 &c, Vector3& outN);

//--

//! Pack 0-1 float value to 0-255 unsigned byte, assumes mapping is linear
static ALWAYS_INLINE uint8_t FloatTo255(float col);

//! Pack 0-1 float value to 0-255 unsigned byte assuming the target format is SRGB (like most textures)
extern BM_CORE_MATH_API uint8_t FloatTo255_SRGB(float linear);

//! Unpack 0-255 unsigned byte to 0-1 floating point value, assumes mapping is linear
static ALWAYS_INLINE float FloatFrom255(uint8_t col);

//! Unpack 0-255 unsigned byte SRGB value to 0-1 float
extern BM_CORE_MATH_API float FloatFrom255_SRGB(uint8_t col);

//--

//! Pack 0-1 float value to 0-65535 unsigned short, assumes mapping is linear
static ALWAYS_INLINE uint16_t FloatTo65535(float col);

//! Pack 0-1 float value to 0-65535 unsigned byte assuming the target format is SRGB (like most textures)
extern BM_CORE_MATH_API uint16_t FloatTo65535_SRGB(float linear);

//! Unpack 0-65535 unsigned short to 0-1 floating point value, assumes mapping is linear
static ALWAYS_INLINE float FloatFrom65535(uint16_t col);

//! Unpack 0-255 unsigned byte SRGB value to 0-1 float
extern BM_CORE_MATH_API float FloatFrom65535_SRGB(uint16_t col);

//--

//! Convert from SRGB to linear space, for values 0-1
static ALWAYS_INLINE float FloatFromSRGBToLinear(float x);

//! Convert from linear space to SRGB space, for values 0-1
static ALWAYS_INLINE float FloatFromLinearToSRGB(float x);

//--

//! lerp float values
template< typename T >
ALWAYS_INLINE T Lerp(T a, T b, float frac);

//! snap value to a grid
template< typename T >
ALWAYS_INLINE T Snap(T val, T grid);

//! min of 3 values
template< typename T >
ALWAYS_INLINE T Min3(T a, T b, T c);

//! max of 3 values
template< typename T >
ALWAYS_INLINE T Max3(T a, T b, T c);

//--

END_INFERNO_NAMESPACE()
