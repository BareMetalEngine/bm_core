/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "curve.h"

BEGIN_INFERNO_NAMESPACE()

//---

/// Unpacked curve control point
class BM_CORE_MATH_API UnpackedCurvePoint : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(UnpackedCurvePoint, IObject);

public:
    UnpackedCurvePoint();
    UnpackedCurvePoint(float _time, float _value, CurveSegmentType type = CurveSegmentType::Linear, CurveBezierHandleType handles = CurveBezierHandleType::Free);

    //---

    CurveSegmentType type = CurveSegmentType::Linear;
    CurveBezierHandleType handles = CurveBezierHandleType::Free;

    float time = 0.0f;
    float value = 0.0f;

    Vector2 c0; // relative to point
    Vector2 c1; // relative to point

    //---
};

typedef RefPtr<UnpackedCurvePoint> UnpackedCurvePointPtr;

//---

/// Unpacked, editable curve
class BM_CORE_MATH_API UnpackedCurve : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(UnpackedCurve, IObject);

public:
    UnpackedCurve(const Curve& source);
    UnpackedCurve(bool looped=false);

    //--

    INLINE bool looped() const { return m_looped; }

    INLINE const Array<UnpackedCurvePointPtr>& points() const { return m_points; }

    //--

    // attach point to curve
    void attachPoint(UnpackedCurvePoint* point);

    // detach point from curve
    void dettachPoint(UnpackedCurvePoint* point);

    // find point by ID
    UnpackedCurvePoint* findPoint(uint32_t id) const;

    //--

    // interpolate value
    float interpolate(float time, int* pointIndex = nullptr) const;

    //--

    // pack into a curve
    Curve pack(float minPointSeparation = 0.001f) const;

    // convert to looped curve
    void makeLooped(bool looped=true);

    //--

    // post-move reorder
    void reorderPoints();

protected:
    Array<UnpackedCurvePointPtr> m_points; // always ordered by time
    bool m_looped = false;

    void fixupLoopedPoint();
};

//---

END_INFERNO_NAMESPACE()
