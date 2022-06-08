/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

enum class CurveSegmentType : uint8_t
{
    Constant, // segment retains constant value till the next segment
    Linear, // interpolate in a linear way between this segment and the next
    Hermite, // interpolate using hermite interpolation
    Bezier, //  interpolate using cubic Bezier, allows 2 additional control points to be edited
};

enum class CurveBezierHandleType : uint8_t
{
    Free,
    Aligned,
};

//---

/// Packed curve data, contains list of timed points with single scalar value that can be interpolated using various modes
/// NOTE: data in packed curve is packed into one linear memory block for efficiency
/// NOTE: only way to create/edit curve is to Unpack and Pack it, use the UnpackedCurve for that (NOTE: it's going to be slow as this is editor friendly object)
class BM_CORE_MATH_API Curve
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(Curve);

public:
    struct RawPoint
    {
        float time = 0.0f;
        float value = 0.0f;

        CurveSegmentType type = CurveSegmentType::Linear;
        CurveBezierHandleType handles = CurveBezierHandleType::Free;

        Vector2 c0;
        Vector2 c1;
    };

    Curve();
    Curve(float defaultvalue, bool looped=false);
    Curve(uint32_t numPoints, const RawPoint* points, bool looped);

    Curve(const Curve& other);
    Curve(Curve&& other);
    Curve& operator=(const Curve& other);
    Curve& operator=(Curve&& other);
    ~Curve();

    //--

    // clear curve to empty curve (returns zero)
    void clear();

    // reset curve to curve with default value
    void reset(float defaultValue = 0.0f, bool looped = false);

    // initialize curve from table of points
    void init(uint32_t numPoints, const RawPoint* points, bool looped = false);

    //--

    // is the curve empty ?
    INLINE bool empty() const { return m_data.empty(); }

    // get number of points in the curve
    uint32_t points() const;

    // is this curve looped ?
    bool looped() const;

    // interpolate value for this curve
    float interpolate(float time) const;

    // unpack curve points
    void unpack(Array<RawPoint>& outPoints) const;

    //--

    // correct Bezier handles so they are not forming a loop
    static bool CorrectBezzierHandles(float v1, float v2, float v3, float v4, float& outScale1, float& outScale2);

    // solve cubic equation, returns number of roots, roots are written to "o"
    static int SolveCubic(double c0, double c1, double c2, double c3, float* o);

    // find time value for which the bezier value equals the "x"
    static int FindZero(float x, float q0, float q1, float q2, float q3, float* o);

    // interpolate bezier value
    static float InterpolateBezier(float f1, float f2, float f3, float f4, float t);

    //--

protected:
    struct Point
    {
        float time = 0.0f;
        float invTimeToNext = 0.0f; // 1/time to next
        uint32_t type:4; // CurveSegmentType
        uint32_t handles : 4; // CurveBezierHandleType
        uint32_t valueOffset:24; // Offset to values (from header base)
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

    //--
};

//---

END_INFERNO_NAMESPACE()
