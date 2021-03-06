/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\vector3 #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

INLINE Vector3::Vector3()
    : x(0)
    , y(0)
    , z(0)
{}

INLINE Vector3::Vector3(float inX, float inY, float inZ)
    : x(inX)
    , y(inY)
    , z(inZ)
{}

INLINE float Vector3::minValue() const
{
    return std::min(x, std::min(y, z));
}

INLINE float Vector3::maxValue() const
{
    return std::max(x, std::max(y, z));
}

INLINE float Vector3::sum() const
{
    return x + y + z;
}

INLINE float Vector3::trace() const
{
    return x * y * z;
}

INLINE Vector3 Vector3::abs() const
{
    return Vector3(std::abs(x), std::abs(y), std::abs(z));
}

INLINE Vector3 Vector3::round() const
{
    return Vector3(std::round(x), std::round(y), std::round(z));
}

INLINE Vector3 Vector3::frac() const
{
    return Vector3(x - std::trunc(x), y - std::trunc(y), z - std::trunc(z));
}

INLINE Vector3 Vector3::trunc() const
{
    return Vector3(std::trunc(x), std::trunc(y), std::trunc(z));
}

INLINE Vector3 Vector3::floor() const
{
    return Vector3(std::floor(x), std::floor(y), std::floor(z));
}

INLINE Vector3 Vector3::ceil() const
{
    return Vector3(std::ceil(x), std::ceil(y), std::ceil(z));
}

INLINE float Vector3::squareLength() const
{
    return x*x + y*y + z*z;
}

INLINE float Vector3::length() const
{
#ifdef PLATFORM_PROSPERO
    return std::hypot3(x, y, z);
#else
    return std::hypot(x, y, z);
#endif
}

INLINE float Vector3::invLength() const
{
    auto len = length();
    return (len > 0.0f) ? (1.0f / len) : 0.0f;
}

INLINE bool Vector3::isNearZero(const  float epsilon)  const
{
    return (x >= -epsilon && x <= epsilon) && (y >= -epsilon && y <= epsilon) && (z >= -epsilon && z <= epsilon);
}

INLINE bool Vector3::isSimilar(const Vector3 &other, float epsilon) const
{
    float dx = x - other.x;
    float dy = y - other.y;
    float dz = z - other.z;
    return (dx >= -epsilon && dx <= epsilon) && (dy >= -epsilon && dy <= epsilon) && (dz >= -epsilon && dz <= epsilon);
}

INLINE bool Vector3::isZero() const
{
    return !x && !y && !z;
}

INLINE float Vector3::normalize()
{
#ifdef PLATFORM_PROSPERO
    auto len = std::hypot3(x,y,z);
#else
    auto len = std::hypot(x, y, z);
#endif
    if (len > NORMALIZATION_EPSILON)
    {
        x /= len;
        y /= len;
        z /= len;
    }
    return len;
}

INLINE float Vector3::normalizeFast()
{
#ifdef PLATFORM_PROSPERO
    auto len = std::hypot3(x, y, z);
#else
    auto len = std::hypot(x,y,z);
#endif
    auto rcp = 1.0f / len;
    x *= rcp;
    y *= rcp;
    z *= rcp;
    return len;
}

INLINE Vector3 Vector3::normalized() const
{
    Vector3 ret(*this);
    ret.normalize();
    return ret;
}

INLINE Vector3 Vector3::normalizedFast() const
{
    Vector3 ret(*this);
    ret.normalizeFast();
    return ret;
}

INLINE Vector3 Vector3::oneOver() const
{
    return Vector3(x != 0.0f ? 1.0f / x : 0.0f,
                    y != 0.0f ? 1.0f / y : 0.0f,
                    z != 0.0f ? 1.0f / z : 0.0f);
}

INLINE Vector3 Vector3::oneOverFast() const
{
    return Vector3(1.0f / x, 1.0f / y, 1.0f / z);
}

INLINE Vector2 Vector3::xx() const { return Vector2(x,x); }
INLINE Vector2 Vector3::yy() const { return Vector2(y,y); }
INLINE Vector2 Vector3::zz() const { return Vector2(z,z); }
INLINE Vector2 Vector3::yx() const { return Vector2(y,x); }
INLINE Vector2 Vector3::xz() const { return Vector2(x,z); }
INLINE Vector2 Vector3::zx() const { return Vector2(z, x); }
INLINE Vector2 Vector3::zy() const { return Vector2(z, y); }

INLINE Vector3 Vector3::xxx() const { return Vector3(x,x,x); }
INLINE Vector3 Vector3::xxy() const { return Vector3(x,x,y); }
INLINE Vector3 Vector3::xxz() const { return Vector3(x,x,z); }
INLINE Vector3 Vector3::xyx() const { return Vector3(x,y,x); }
INLINE Vector3 Vector3::xyy() const { return Vector3(x,y,y); }
INLINE Vector3 Vector3::xzx() const { return Vector3(x,z,x); }
INLINE Vector3 Vector3::xzy() const { return Vector3(x,z,y); }
INLINE Vector3 Vector3::xzz() const { return Vector3(x,z,z); }
INLINE Vector3 Vector3::yxx() const { return Vector3(y,x,x); }
INLINE Vector3 Vector3::yxy() const { return Vector3(y,x,y); }
INLINE Vector3 Vector3::yxz() const { return Vector3(y,x,z); }
INLINE Vector3 Vector3::yyx() const { return Vector3(y,y,x); }
INLINE Vector3 Vector3::yyy() const { return Vector3(y,y,y); }
INLINE Vector3 Vector3::yyz() const { return Vector3(y,y,z); }
INLINE Vector3 Vector3::yzx() const { return Vector3(y,z,x); }
INLINE Vector3 Vector3::yzy() const { return Vector3(y,z,y); }
INLINE Vector3 Vector3::yzz() const { return Vector3(y,z,z); }
INLINE Vector3 Vector3::zxx() const { return Vector3(z,x,x); }
INLINE Vector3 Vector3::zxy() const { return Vector3(z,x,y); }
INLINE Vector3 Vector3::zxz() const { return Vector3(z,x,z); }
INLINE Vector3 Vector3::zyx() const { return Vector3(z,y,x); }
INLINE Vector3 Vector3::zyy() const { return Vector3(z,y,y); }
INLINE Vector3 Vector3::zyz() const { return Vector3(z,y,z); }
INLINE Vector3 Vector3::zzx() const { return Vector3(z,z,x); }
INLINE Vector3 Vector3::zzy() const { return Vector3(z,z,y); }
INLINE Vector3 Vector3::zzz() const { return Vector3(z,z,z); }

INLINE const Vector2& Vector3::xy() const { return *(const Vector2*) &x; }
INLINE const Vector2& Vector3::yz() const { return *(const Vector2*) &y; }
INLINE const Vector3& Vector3::xyz() const { return *this; }

INLINE Vector2& Vector3::xy() { return *(Vector2*) &x; }
INLINE Vector2& Vector3::yz() { return *(Vector2*) &y; }
INLINE Vector3& Vector3::xyz()  { return *this; }

INLINE Vector2 Vector3::_xy() const { return xy(); }
INLINE Vector2 Vector3::_yz() const { return yz(); }
INLINE Vector3 Vector3::_xyz() const { return xyz(); }

INLINE Vector4 Vector3::xyzw(float w) const
{
    return Vector4(x,y,z,w);
}

INLINE Vector3 Vector3::transform(const Vector3& vx, const Vector3& vy, const Vector3& vz) const
{
    return (x * vx) + (y * vy) + (z * vz);
}

INLINE float Vector3::distance(const Vector3 &other) const
{
#ifdef PLATFORM_PROSPERO
    return std::hypot3(x - other.x, y - other.y, z - other.z);
#else
    return std::hypot(x - other.x, y - other.y, z - other.z);
#endif
}

INLINE float Vector3::squareDistance(const Vector3 &other) const
{
    return (x - other.x) * (x - other.x) + (y - other.y)*(y - other.y) + (z - other.z)*(z - other.z);
}

INLINE Vector3 Vector3::operator-() const
{
    return Vector3(-x, -y, -z);
}

INLINE Vector3 Vector3::operator+(const Vector3 &other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

INLINE Vector3 Vector3::operator+(float value) const
{
    return Vector3(x + value, y + value, z + value);
}

INLINE Vector3 Vector3::operator-(const Vector3 &other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

INLINE Vector3 Vector3::operator-(float value) const
{
    return Vector3(x - value, y - value, z - value);
}

INLINE Vector3 Vector3::operator*(const Vector3 &other) const
{
    return Vector3(x*other.x, y*other.y, z*other.z);
}

INLINE Vector3 Vector3::operator*(float value) const
{
    return Vector3(x*value, y*value, z*value);
}

INLINE Vector3 operator*(float value, const Vector3 &other)
{
    return Vector3(value*other.x, value*other.y, value*other.z);
}

INLINE Vector3 Vector3::operator/(const Vector3 &other) const
{
    return Vector3(x / other.x, y / other.y, z / other.z);
}

INLINE Vector3 Vector3::operator/(float value) const
{
    return Vector3(x / value, y / value, z / value);
}

INLINE Vector3& Vector3::operator+=(const Vector3 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

INLINE Vector3& Vector3::operator+=(float value)
{
    x += value;
    y += value;
    z += value;
    return *this;
}

INLINE Vector3& Vector3::operator-=(const Vector3 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

INLINE Vector3& Vector3::operator-=(float value)
{
    x -= value;
    y -= value;
    z -= value;
    return *this;
}

INLINE Vector3& Vector3::operator*=(const Vector3 &other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

INLINE Vector3& Vector3::operator*=(float value)
{
    x *= value;
    y *= value;
    z *= value;
    return *this;
}

INLINE Vector3& Vector3::operator/=(const Vector3 &other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

INLINE Vector3& Vector3::operator/=(float value)
{
    x /= value;
    y /= value;
    z /= value;
    return *this;
}

INLINE bool Vector3::operator==(const Vector3 &other) const
{
    return x == other.x && y == other.y && z == other.z;
}

INLINE bool Vector3::operator!=(const Vector3 &other) const
{
    return x != other.x || y != other.y || z != other.z;
}

INLINE float Vector3::operator|(const Vector3 &other) const
{
    return (x*other.x) + (y*other.y) + (z*other.z);
}

INLINE Vector3 Vector3::operator^(const Vector3 &other) const
{
    return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

INLINE float Vector3::operator[](int index) const
{
    return ((const float*)this)[index];
}

INLINE float& Vector3::operator[](int index)
{
    return ((float*)this)[index];
}

INLINE int Vector3::largestAxis() const
{
    float ax = std::abs(x);
    float ay = std::abs(y);
    float az = std::abs(z);

    if (ax > ay && ax > az) return 0;
    if (ay > ax && ay > az) return 1;
    return 2;
}

INLINE int Vector3::smallestAxis() const
{
    float ax = std::abs(x);
    float ay = std::abs(y);
    float az = std::abs(z);

    if (ax < ay && ax < az) return 0;
    if (ay < ax && ay < az) return 1;
    return 2;
}

//--

INLINE Vector3 Vector3::normalPart(const Vector3& normal) const
{
    return normal * dot(normal);;
}

INLINE Vector3 Vector3::tangentPart(const Vector3& normal) const
{
    return *this - normalPart(normal);
}

INLINE std::pair<Vector3, Vector3> Vector3::decompose(const Vector3& normal) const
{
    auto np = normal * dot(normal);;
    return std::make_pair(np, *this - np);
}

INLINE float Vector3::dot(const Vector3& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

INLINE void Vector3::clampLength(float val)
{
    auto sqlen = squareLength();
    if (sqlen > (val * val))
    {
        auto len = std::sqrt(sqlen);
        if (len > FLT_EPSILON)
        {
            auto scale = val / len;
            x *= scale;
            y *= scale;
            z *= scale;
        }
    }
}

INLINE Vector3 Vector3::clampedLength(float length) const
{
    Vector3 ret(*this);
    ret.clampLength(length);
    return ret;
}

//--

INLINE void Vector3::snap(float grid)
{
    x = Snap(x, grid);
    y = Snap(y, grid);
    z = Snap(z, grid);
}

INLINE Vector3 Vector3::snapped(float grid) const
{
    return Vector3(
        Snap(x, grid),
        Snap(y, grid),
        Snap(z, grid));
}

INLINE Vector3 Vector3::min(const Vector3& b) const
{
    return Vector3(
        std::min(x, b.x),
        std::min(y, b.y),
        std::min(z, b.z));
}

INLINE Vector3 Vector3::minScalar(float val) const
{
    return Vector3(
        std::min(x, val),
        std::min(y, val),
        std::min(z, val));
}

INLINE Vector3 Vector3::max(const Vector3& b) const
{
    return Vector3(
        std::max(x, b.x),
        std::max(y, b.y),
        std::max(z, b.z));
}

INLINE Vector3 Vector3::maxScalar(float val) const
{
    return Vector3(
        std::max(x, val),
        std::max(y, val),
        std::max(z, val));
}

INLINE Vector3 Vector3::clamped(const Vector3& minV, const Vector3& maxV) const
{
    return Vector3(
        std::clamp(x, minV.x, maxV.x),
        std::clamp(y, minV.y, maxV.y),
        std::clamp(z, minV.z, maxV.z));
}

INLINE Vector3 Vector3::clampedScalar(float minV, float maxV) const
{
    return Vector3(
        std::clamp(x, minV, maxV),
        std::clamp(y, minV, maxV),
        std::clamp(z, minV, maxV));
}

INLINE void Vector3::clamp(const Vector3& minV, const Vector3& maxV)
{
    x = std::clamp(x, minV.x, maxV.x);
    y = std::clamp(y, minV.y, maxV.y);
    z = std::clamp(z, minV.z, maxV.z);
}

INLINE void Vector3::clampScalar(float minV, float maxV)
{
    x = std::clamp(x, minV, maxV);
    y = std::clamp(y, minV, maxV);
    z = std::clamp(z, minV, maxV);
}

INLINE Vector3 Vector3::lerp(const Vector3& target, float f) const
{
    return Vector3(
        x + (target.x - x) * f,
        y + (target.y - y) * f,
        z + (target.z - z) * f);
}

INLINE Vector3 Vector3::lerpPerComponent(const Vector3& target, const Vector3& f) const
{
    return Vector3(
        x + (target.x - x) * f.x,
        y + (target.y - y) * f.y,
        z + (target.z - z) * f.z);
}

//--

END_INFERNO_NAMESPACE()