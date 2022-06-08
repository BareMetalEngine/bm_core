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

ALWAYS_INLINE uint8_t FloatTo255(float val)
{
    return (uint8_t)std::clamp(val * 255.0f, 0.0f, 255.0f);
}

ALWAYS_INLINE uint16_t FloatTo65535(float val)
{
    return (uint16_t)std::clamp(val * 65535.0f, 0.0f, 65535.0f);
}

ALWAYS_INLINE float FloatFrom255(uint8_t col)
{
    const float inv = 1.0f / 255.0f;
    return col * inv;
}

ALWAYS_INLINE float FloatFrom65535(uint16_t col)
{
    const float inv = 1.0f / 65535;
    return col * inv;
}

//--

ALWAYS_INLINE float FloatFromSRGBToLinear(float x)
{
    if (x <= 0.0f)
        return 0.0f;
    else if (x >= 1.0f)
        return 1.0f;
    else if (x < 0.04045f)
        return x / 12.92f;
    else
        return std::pow((x + 0.055f) / 1.055f, 2.4f);
}

ALWAYS_INLINE float FloatFromLinearToSRGB(float x)
{
    if (x <= 0.0f)
        return 0.0f;
    else if (x >= 1.0f)
        return 1.0f;
    else if (x < 0.0031308f)
        return x * 12.92f;
    else
        return std::pow(x, 1.0f / 2.4f) * 1.055f - 0.055f;
}

//--

template< typename T >
ALWAYS_INLINE T Snap(T val, T grid)
{
    return (grid > 0.0f) ? (std::round(val / grid) * grid) : val;
}

template< typename T >
ALWAYS_INLINE T Lerp(T a, T b, float frac)
{
    return a + (b - a) * frac;
}

template< typename T >
ALWAYS_INLINE float Min3(T a, T b, T c)
{
    return std::min<T>(a, std::min<T>(b, c));
}

template< typename T >
ALWAYS_INLINE T Max3(T a, T b, T c)
{
    return std::max<T >(a, std::max<T >(b, c));
}

//--

END_INFERNO_NAMESPACE();
