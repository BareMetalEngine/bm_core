/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "curve.h"
#include "unpackedCurve.h"

BEGIN_INFERNO_NAMESPACE()

//---

RTTI_BEGIN_TYPE_ENUM(CurveSegmentType);
RTTI_ENUM_OPTION(Constant);
RTTI_ENUM_OPTION(Linear);
RTTI_ENUM_OPTION(Hermite);
RTTI_ENUM_OPTION(Bezier);
RTTI_END_TYPE();

RTTI_BEGIN_TYPE_CLASS(UnpackedCurvePoint);
RTTI_PROPERTY(type);
RTTI_PROPERTY(time);
RTTI_PROPERTY(value);
RTTI_PROPERTY(c0);
RTTI_PROPERTY(c1);
RTTI_END_TYPE();

UnpackedCurvePoint::UnpackedCurvePoint()
{
}

UnpackedCurvePoint::UnpackedCurvePoint(float time_, float value_, CurveSegmentType type_, CurveBezierHandleType handles_)
    : type(type_)
    , handles(handles_)
    , time(time_)
    , value(value_)
{}

//---

RTTI_BEGIN_TYPE_CLASS(UnpackedCurve);
    RTTI_PROPERTY(m_points);
    RTTI_PROPERTY(m_looped);
RTTI_END_TYPE();

UnpackedCurve::UnpackedCurve(const Curve& source)
{
    const auto count = source.points();
    m_points.reserve(count);

    m_looped = source.looped();

    Array<Curve::RawPoint> rawPoints;
    source.unpack(rawPoints);

    for (const auto& data : rawPoints)
    {
        auto point = RefNew<UnpackedCurvePoint>();
        point->type = data.type;
        point->time = data.time;
        point->value = data.value;
        point->c0 = data.c0;
        point->c1 = data.c1;

        m_points.pushBack(point);
        point->parent(this);
    }
}

UnpackedCurve::UnpackedCurve(bool looped)
    : m_looped(looped)
{    
}

//--

void UnpackedCurve::attachPoint(UnpackedCurvePoint* point)
{
    DEBUG_CHECK_RETURN_EX(point, "Invalid point");
    DEBUG_CHECK_RETURN_EX(point->parent() == nullptr, "Point has a parent");
    DEBUG_CHECK_RETURN_EX(!m_points.contains(point), "Point already in the curve");

    m_points.pushBack(AddRef(point));
    point->parent(this);

    reorderPoints();
    markModified();
}

void UnpackedCurve::dettachPoint(UnpackedCurvePoint* point)
{
    DEBUG_CHECK_RETURN_EX(point, "Invalid point");
    DEBUG_CHECK_RETURN_EX(point->parent() == this, "Point not from this curve");
    DEBUG_CHECK_RETURN_EX(m_points.contains(point), "Point not in curve");

    point->parent(nullptr);
    m_points.remove(point);

    markModified();
}

UnpackedCurvePoint* UnpackedCurve::findPoint(uint32_t id) const
{
    for (const auto& p : m_points)
        if (p->id() == id)
            return p;

    return nullptr;
}

//--

float UnpackedCurve::interpolate(float time, int* pointIndex) const
{
    if (m_points.empty())
        return 0.0f;

    if (time <= m_points[0]->time)
    {
        if (pointIndex)
            *pointIndex = 0;

        return m_points[0]->value;
    }
    else if (time >= m_points.back()->time)
    {
        if (pointIndex)
            *pointIndex = m_points.lastValidIndex();

        return m_points.back()->value;
    }

    uint32_t index = 1;
    for (;;)
    {
        if (time <= m_points[index]->time)
            break;
        ++index;
    }

    if (pointIndex)
        *pointIndex = index - 1;

    const auto* pointA = m_points[index-1].get();
    const auto* pointB = m_points[index].get();

    float diff = pointB->time - pointA->time;
    if (diff < 0.001f)
        return pointA->value;

    const float frac = std::min<float>((time - pointA->time) / diff, 1.0f);

    switch (pointA->type)
    {
        case CurveSegmentType::Constant:
        case CurveSegmentType::Hermite:
            break;

        case CurveSegmentType::Linear:
            return pointA->value + ((pointB->value - pointA->value) * frac);

        case CurveSegmentType::Bezier:
        {
            float scale0 = 1.0f;
            float scale1 = 1.0f;

            if (!Curve::CorrectBezzierHandles(pointA->time, pointA->time + pointA->c1.x, pointB->time + pointB->c0.x, pointB->time, scale0, scale1))
            {
                // find the intersection point
                float roots[8];
                roots[0] = 0.0f;
                Curve::FindZero(time, pointA->time, pointA->time + pointA->c1.x, pointB->time + pointB->c0.x, pointB->time, roots);

                // blend values
                return Curve::InterpolateBezier(pointA->value, pointA->value + pointA->c1.y, pointB->value + pointB->c0.y, pointB->value, roots[0]);
            }
            else
            {
                // scale so they don't intersect
                auto tcur = pointA->c1 * scale0;
                auto tnext = pointB->c0 * scale1;

                // find the intersection point
                float roots[8];
                roots[0] = 0.0f;
                Curve::FindZero(time, pointA->time, pointA->time + tcur.x, pointB->time + tnext.x, pointB->time, roots);

                // blend values
                return Curve::InterpolateBezier(pointA->value, pointA->value + tcur.y, pointB->value + tnext.y, pointB->value, roots[0]);
            }
        }
    }

    return pointA->value;
}

//--

void UnpackedCurve::reorderPoints()
{
    std::sort(m_points.begin(), m_points.end(), [](const UnpackedCurvePointPtr& a, const UnpackedCurvePointPtr& b)
        {
            return a->time < b->time;
        });

    fixupLoopedPoint();
}

void UnpackedCurve::fixupLoopedPoint()
{
    if (m_looped && !m_points.empty())
    {
        m_points.back()->value = m_points.front()->value;
    }
}

void UnpackedCurve::makeLooped(bool looped /*= true*/)
{
    if (m_looped != looped)
    {
        m_looped = looped;

        if (looped)
            fixupLoopedPoint();
    }
}

Curve UnpackedCurve::pack(float minPointSeparation) const
{
    InplaceArray<Curve::RawPoint, 128> rawPoints;
    rawPoints.reserve(m_points.size() + 1);

    // copy points, skip if to close
    for (auto i : m_points.indexRange())
    {
        const auto* p = m_points[i].get();

        bool add = true;

        if (!rawPoints.empty())
        {
            DEBUG_CHECK(p->time >= rawPoints.back().time);
            const float dist = p->time - rawPoints.back().time;

            if (dist <= minPointSeparation)
                add = false;
        }

        if (add)
        {
            auto& rawPoint = rawPoints.emplaceBack();
            rawPoint.time = p->time;
            rawPoint.type = p->type;
            rawPoint.value = p->value;
            rawPoint.c0 = p->c0;
            rawPoint.c1 = p->c1;
        }
    }

    // if we are looped copy the value
    if (m_looped && rawPoints.size() >= 2)
    {
        auto& fp = rawPoints.front();
        auto& bp = rawPoints.back();
        bp.value = fp.value;
        bp.type = CurveSegmentType::Constant;
    }

    // create a curve
    return Curve(rawPoints.size(), rawPoints.typedData(), m_looped);
}

//---

END_INFERNO_NAMESPACE()
