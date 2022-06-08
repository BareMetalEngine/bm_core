/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\rotation #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

INLINE Angles::Angles(float inPitch, float inYaw, float inRoll)
{
    pitch = inPitch;
    yaw = inYaw;
    roll = inRoll;
}

INLINE bool Angles::isZero() const
{
    return (pitch == 0.0f) && (yaw == 0.0f) && (roll == 0.0f);
}

INLINE bool Angles::isNearZero(float eps) const
{
    return (std::abs(pitch) <= eps) && (std::abs(yaw) <= eps) && (std::abs(roll) <= eps);
}

INLINE Angles Angles::abs() const
{
    return Angles(std::abs(pitch), std::abs(yaw), std::abs(roll));
}

INLINE float Angles::maxValue() const
{
    return std::max(pitch, std::max(yaw, roll));
}

INLINE float Angles::minValue() const
{
    return std::min(pitch, std::min(yaw, roll));
}

INLINE float Angles::sum() const
{
    return pitch + yaw + roll;
}

INLINE uint8_t Angles::smallestAxis() const
{
    float ax = std::abs(roll);
    float ay = std::abs(pitch);
    float az = std::abs(yaw);

    if (ax < ay && ax < az) return 0;
    if (ay < ax && ay < az) return 1;
    return 2;
}

INLINE uint8_t Angles::largestAxis() const
{
    float ax = std::abs(roll);
    float ay = std::abs(pitch);
    float az = std::abs(yaw);

    if (ax > ay && ax > az) return 0;
    if (ay > ax && ay > az) return 1;
    return 2;
}

INLINE bool Angles::operator==(const Angles &other) const
{
    return pitch == other.pitch && yaw == other.yaw && roll == other.roll;
}

INLINE bool Angles::operator!=(const Angles &other) const
{
    return pitch != other.pitch || yaw != other.yaw || roll != other.roll;
}

INLINE Angles Angles::operator+(const Angles &other) const
{
    return Angles(pitch + other.pitch, yaw + other.yaw, roll + other.roll);
}

INLINE Angles& Angles::operator+=(const Angles &other)
{
    pitch = pitch + other.pitch;
    yaw = yaw + other.yaw;
    roll = roll + other.roll;
    return *this;
}

INLINE Angles Angles::operator-() const
{
    return Angles(-pitch, -yaw, -roll);
}

INLINE Angles Angles::operator-(const Angles &other) const
{
    return Angles(pitch - other.pitch, yaw - other.yaw, roll - other.roll);
}

INLINE Angles& Angles::operator-=(const Angles &other)
{
    pitch = pitch - other.pitch;
    yaw = yaw - other.yaw;
    roll = roll - other.roll;
    return *this;
}

INLINE Angles& Angles::operator*=(float value)
{
    pitch = pitch * value;
    yaw = yaw * value;
    roll = roll * value;
    return *this;
}

INLINE Angles Angles::operator*(float value) const
{
    return Angles(pitch*value, yaw*value, roll*value);
}

INLINE Angles& Angles::operator/=(float value)
{
    pitch = pitch / value;
    yaw = yaw / value;
    roll = roll / value;
    return *this;
}

INLINE Angles Angles::operator/(float value) const
{
    return Angles(pitch / value, yaw / value, roll / value);
}

//--

INLINE void Angles::snap(float grid)
{
    pitch = Snap(pitch, grid);
    yaw = Snap(yaw, grid);
    roll = Snap(roll, grid);
}

INLINE Angles Angles::snapped(float grid) const
{
    return Angles(
        Snap(pitch, grid),
        Snap(yaw, grid),
        Snap(roll, grid));
}

INLINE float Angles::dot(const Angles& other) const
{
    return forward() | other.forward();
}

INLINE float Angles::dotAxis(const Vector3& other) const
{
    return forward() | other;
}

//--

INLINE Angles Angles::min(const Angles& other) const
{
    return Angles(
        std::min(pitch, other.pitch),
        std::min(yaw, other.yaw),
        std::min(roll, other.roll));
}

INLINE Angles Angles::minScalar(float val) const
{
    return Angles(
        std::min(pitch, val),
        std::min(yaw, val),
        std::min(roll, val));
}

INLINE Angles Angles::max(const Angles& other) const
{
    return Angles(
        std::max(pitch, other.pitch),
        std::max(yaw, other.yaw),
        std::max(roll, other.roll));
}

INLINE Angles Angles::maxScalar(float val) const
{
    return Angles(
        std::max(pitch, val),
        std::max(yaw, val),
        std::max(roll, val));
}

INLINE Angles Angles::clamped(const Angles& minV, const Angles& maxV) const
{
    return Angles(
        std::clamp(pitch, minV.pitch, maxV.pitch),
        std::clamp(yaw, minV.yaw, maxV.yaw),
        std::clamp(roll, minV.roll, maxV.roll));
}

INLINE Angles Angles::clampedScalar(float minV, float maxV) const
{
    return Angles(
        std::clamp(pitch, minV, maxV),
        std::clamp(yaw, minV, maxV),
        std::clamp(roll, minV, maxV));
}

INLINE void Angles::clamp(const Angles& minV, const Angles& maxV)
{
    pitch = std::clamp(pitch, minV.pitch, maxV.pitch);
    yaw = std::clamp(yaw, minV.yaw, maxV.yaw);
    roll = std::clamp(roll, minV.roll, maxV.roll);
}

INLINE void Angles::clampScalar(float minV, float maxV)
{
    pitch = std::clamp(pitch, minV, maxV);
    yaw = std::clamp(yaw, minV, maxV);
    roll = std::clamp(roll, minV, maxV);
}

//--

END_INFERNO_NAMESPACE()
