/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// Basic color gradient, stores linear colors
class BM_CORE_MATH_API ColorGradient
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(ColorGradient);

public:
    struct RawPoint
    {
        float time = 0.0f;
        Vector3 color;
    };

    ColorGradient();
    ColorGradient(const Vector3& defaultvalue, bool looped = false);
    ColorGradient(uint32_t numPoints, const RawPoint* points, bool looped); // W = time

    ColorGradient(const ColorGradient& other);
    ColorGradient(ColorGradient&& other);
    ColorGradient& operator=(const ColorGradient& other);
    ColorGradient& operator=(ColorGradient&& other);
    ~ColorGradient();

    //--
    // 
    // clear gradient to empty curve (returns zero)
    void clear();

    // reset color gradient to a single value
    void reset(const Vector3& defaultColor = Vector3::ONE(), bool looped = false);

    // initialize color gradient from a table
    void init(uint32_t numPoints, const RawPoint* points, bool looped = false);

    //--

    // is the curve empty ?
    INLINE bool empty() const { return m_data.empty(); }

    // get number of points in the curve
    uint32_t points() const;

    // is this curve looped ?
    bool looped() const;

    // interpolate value for this curve
    Vector3 interpolate(float time) const;

    // unpack curve points
    void unpack(Array<RawPoint>& outPoints) const;

    //--

private:
    struct Point
    {
        float time = 0.0f;
        float invTimeToNext = 0.0f; // 1/time to next
        uint16_t valueOffset = 0;
    };

    struct Header
    {
        uint8_t loopFlag = 0; // if set the last key value matches the first key value (this is editor flag)
        uint16_t numPoints = 0;
        float minTime = 0.0f;
        float maxTime = 0.0f;
        float invTimeSpan = 0.0f; // 1/(maxTime-minTime), for looping

        Point points[1]; // extends into buffer
    };

    ALWAYS_INLINE const Header* header() const { return (const Header*)m_data.data(); }

    Buffer m_data;
};

//--

END_INFERNO_NAMESPACE()
