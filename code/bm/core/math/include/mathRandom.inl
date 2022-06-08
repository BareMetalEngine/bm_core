/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

ALWAYS_INLINE Vector2 Random::unit2()
{
    return Vector2(unit(), unit());
}

ALWAYS_INLINE Vector3 Random::unit3()
{
    return Vector3(unit(), unit(), unit());
}

ALWAYS_INLINE Vector4 Random::unit4()
{
    return Vector4(unit(), unit(), unit(), unit());
}

ALWAYS_INLINE float Random::range(float min, float max)
{
    return unit() * (max - min) + min;
}

ALWAYS_INLINE double Random::rangeDouble(double min, double max)
{
    return unitDouble() * (max - min) + min;
}

ALWAYS_INLINE uint32_t Random::select(uint32_t max)
{
    if (max <= 1)
        return 0;

    auto big = (uint64_t)next() * max;
    return (uint32_t)(big >> 32);
}

///--

END_INFERNO_NAMESPACE()
