/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\vector2 #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

INLINE Vector2::Vector2()
    : x(0), y(0)
{}

INLINE Vector2::Vector2(float inX, float inY)
    : x(inX), y(inY)
{}

INLINE float Vector2::minValue() const
{
    return std::min(x, y);
}

INLINE float Vector2::maxValue() const
{
    return std::max(x, y);
}

INLINE float Vector2::sum() const
{
    return x + y;
}

INLINE float Vector2::trace() const
{
    return x * y;
}

INLINE Vector2 Vector2::oneOver() const
{
    return Vector2(x != 0.0f ? 1.0f / x : 0.0f, y != 0.0f ? 1.0f / y : 0.0f);
}

INLINE Vector2 Vector2::oneOverFast() const
{
    return Vector2(1.0f / x, 1.0f / y);
}

INLINE Vector2 Vector2::abs() const
{
    return Vector2(std::abs(x), std::abs(y));
}

INLINE Vector2 Vector2::round() const
{
    return Vector2(std::round(x), std::round(y));
}

INLINE Vector2 Vector2::frac() const
{
    return Vector2(x - std::trunc(x), y - std::trunc(y));
}

INLINE Vector2 Vector2::trunc() const
{
    return Vector2(std::trunc(x), std::trunc(y));
}

INLINE Vector2 Vector2::floor() const
{
    return Vector2(std::floor(x), std::floor(y));
}

INLINE Vector2 Vector2::ceil() const
{
    return Vector2(std::ceil(x), std::ceil(y));
}

INLINE float Vector2::squareLength() const
{
    return x * x + y * y;
}

INLINE float Vector2::length() const
{
    return std::hypot(x, y);
}

INLINE float Vector2::invLength() const
{
    auto len  = length();
    return (len > 0.0f) ? (1.0f / len) : 0.0f;
}

INLINE bool Vector2::isNearZero(float epsilon) const
{
    return (x >= -epsilon && x <= epsilon) && (y >= -epsilon && y <= epsilon);
}

INLINE bool Vector2::isSimilar(const Vector2 &other, float epsilon) const
{
    float dx = x - other.x;
    float dy = y - other.y;
    return (dx >= -epsilon && dx <= epsilon) && (dy >= -epsilon && dy <= epsilon);
}

INLINE bool Vector2::isZero() const
{
    return !x && !y;
}

INLINE float Vector2::normalize()
{
    float length = std::hypot(x, y);
    if (length > NORMALIZATION_EPSILON)
    {
        x /= length;
        y /= length;
    }
    return length;
}

INLINE Vector2 Vector2::normalized() const
{
    Vector2 ret(*this);
    ret.normalize();
    return ret;
}

INLINE float Vector2::normalizeFast()
{
    float length = std::hypot(x, y);
    float oneOver = 1.0f / length;
    x *= oneOver;
    y *= oneOver;
    return length;
}

INLINE Vector2 Vector2::normalizedFast() const
{
    Vector2 ret(*this);
    ret.normalizeFast();
    return ret;
}

INLINE Vector2 Vector2::prep() const
{
    return Vector2(y, -x);
}

INLINE float Vector2::distance(const Vector2 &other) const
{
    return std::hypot(x - other.x, y - other.y);
}

INLINE float Vector2::squareDistance(const Vector2 &other) const
{
    return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
}

INLINE int Vector2::largestAxis() const
{
    return std::abs(y) > std::abs(x) ? 1 : 0;
}

INLINE int Vector2::smallestAxis() const
{
    return std::abs(y) < std::abs(x) ? 1 : 0;
}

INLINE Vector2 Vector2::operator-() const
{
    return Vector2(-x, -y);
}

INLINE Vector2 Vector2::operator+(const Vector2 &other) const
{
    return Vector2(x + other.x, y + other.y);
}

INLINE Vector2 Vector2::operator+(float value) const
{
    return Vector2(x + value, y + value);
}

INLINE Vector2 Vector2::operator-(const Vector2 &other) const
{
    return Vector2(x - other.x, y - other.y);
}

INLINE Vector2 Vector2::operator-(float value) const
{
    return Vector2(x - value, y - value);
}

INLINE Vector2 Vector2::operator*(const Vector2 &other) const
{
    return Vector2(x * other.x, y * other.y);
}

INLINE Vector2 Vector2::operator*(float value) const
{
    return Vector2(x * value, y * value);
}

INLINE Vector2 operator*(float value, const Vector2 &other)
{
    return Vector2(value * other.x, value * other.y);
}

INLINE Vector2 Vector2::operator/(const Vector2 &other) const
{
    return Vector2(x / other.x, y / other.y);
}

INLINE Vector2 Vector2::operator/(float value) const
{
    return Vector2(x / value, y / value);
}

INLINE Vector2 &Vector2::operator+=(const Vector2 &other)
{
    x += other.x;
    y += other.y;
    return *this;
}

INLINE Vector2 &Vector2::operator+=(float value)
{
    x += value;
    y += value;
    return *this;
}

INLINE Vector2 &Vector2::operator-=(const Vector2 &other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

INLINE Vector2 &Vector2::operator-=(float value)
{
    x -= value;
    y -= value;
    return *this;
}

INLINE Vector2 &Vector2::operator*=(const Vector2 &other)
{
    x *= other.x;
    y *= other.y;
    return *this;
}

INLINE Vector2 &Vector2::operator*=(float value)
{
    x *= value;
    y *= value;
    return *this;
}

INLINE Vector2 &Vector2::operator/=(const Vector2 &other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}

INLINE Vector2 &Vector2::operator/=(float value)
{
    x /= value;
    y /= value;
    return *this;
}

INLINE bool Vector2::operator==(const Vector2 &other) const
{
    return x == other.x && y == other.y;
}

INLINE bool Vector2::operator!=(const Vector2 &other) const
{
    return x != other.x || y != other.y;
}

INLINE float Vector2::operator|(const Vector2 &other) const
{
    return (x * other.x) + (y * other.y);
}

INLINE Vector2 Vector2::operator~() const
{
    return prep();
}

INLINE Vector2 Vector2::xx() const
{
    return Vector2(x, x);
}

INLINE Vector2 Vector2::yy() const
{
    return Vector2(y, y);
}

INLINE Vector2 Vector2::yx() const
{
    return Vector2(y, x);
}

INLINE const Vector2& Vector2::xy() const
{
    return *this;
}

INLINE Vector2& Vector2::xy()
{
    return *this;
}

INLINE Vector2 Vector2::_xy() const
{
    return xy();
}

INLINE Vector3 Vector2::xyz(float z) const
{
    return Vector3(x, y, z);
}

INLINE Vector4 Vector2::xyzw(float z, float w) const
{
    return Vector4(x, y, z, w);
}

//--

INLINE Vector2 Vector2::normalPart(const Vector2& normal) const
{
    return normal * dot(normal);;
}

INLINE Vector2 Vector2::tangentPart(const Vector2& normal) const
{
    return *this - normalPart(normal);
}

INLINE std::pair<Vector2, Vector2> Vector2::decompose(const Vector2& normal) const
{
    auto np = normal * dot(normal);;
    return std::make_pair(np, *this - np);
}

INLINE float Vector2::dot(const Vector2& other) const
{
    return (x * other.x) + (y * other.y);
}

INLINE void Vector2::clampLength(float val)
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
        }
    }
}

INLINE Vector2 Vector2::clampedLength(float length) const
{
    Vector2 ret(*this);
    ret.clampLength(length);
    return ret;
}

//--

INLINE Vector2 Vector2::min(const Vector2& other) const
{
    return Vector2(std::min(x, other.x), std::min(y, other.y));
}

INLINE Vector2 Vector2::minScalar(float val) const
{
    return Vector2(std::min(x, val), std::min(y, val));
}

INLINE Vector2 Vector2::max(const Vector2& other) const
{
    return Vector2(std::max(x, other.x), std::max(y, other.y));
}

INLINE Vector2 Vector2::maxScalar(float val) const
{
    return Vector2(std::max(x, val), std::max(y, val));
}

INLINE Vector2 Vector2::lerp(const Vector2& target, float f) const
{
    return Vector2(
        x + (target.x - x) * f,
        y + (target.y - y) * f);
}

INLINE Vector2 Vector2::lerpPerComponent(const Vector2& target, const Vector2& f) const
{
    return Vector2(
        x + (target.x - x) * f.x,
        y + (target.y - y) * f.y);
}

INLINE void Vector2::snap(float grid)
{
    x = Snap(x, grid);
    y = Snap(y, grid);
}

INLINE Vector2 Vector2::snapped(float grid) const
{
    return Vector2(
        Snap(x, grid),
        Snap(y, grid));
}

INLINE Vector2 Vector2::clamped(const Vector2& minV, const Vector2& maxV) const
{
    return Vector2(
        std::clamp(x, minV.x, maxV.x),
        std::clamp(y, minV.y, maxV.y));
}

INLINE Vector2 Vector2::clampedScalar(float minV, float maxV) const
{
    return Vector2(
        std::clamp(x, minV, maxV),
        std::clamp(y, minV, maxV));
}

INLINE void Vector2::clamp(const Vector2& minV, const Vector2& maxV)
{
    x = std::clamp(x, minV.x, maxV.x);
    y = std::clamp(y, minV.y, maxV.y);
}

INLINE void Vector2::clampScalar(float minV, float maxV)
{
    x = std::clamp(x, minV, maxV);
    y = std::clamp(y, minV, maxV);
}

//--

END_INFERNO_NAMESPACE()
