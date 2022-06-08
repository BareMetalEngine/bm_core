/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "colorGradient.h"

BEGIN_INFERNO_NAMESPACE()

//---

RTTI_BEGIN_TYPE_STRUCT(ColorGradient);
    RTTI_TYPE_TRAIT().zeroInitializationValid();
    RTTI_PROPERTY(m_data);
RTTI_END_TYPE();

//---

ColorGradient::ColorGradient() = default;
ColorGradient::ColorGradient(const ColorGradient& other) = default;
ColorGradient& ColorGradient::operator=(const ColorGradient& other) = default;
ColorGradient::~ColorGradient() = default;
ColorGradient::ColorGradient(ColorGradient && other) = default;
ColorGradient& ColorGradient::operator=(ColorGradient && other) = default;

ColorGradient::ColorGradient(const Vector3& defaultvalue, bool looped)
{
    reset(defaultvalue, looped);
}

ColorGradient::ColorGradient(uint32_t numPoints, const RawPoint* points, bool looped)
{
    init(numPoints, points, looped);
}

void ColorGradient::clear()
{
    m_data.reset();
}

void ColorGradient::init(uint32_t numPoints, const RawPoint* unfilteredPoints, bool looped)
{
    clear();

    if (numPoints > 0)
    {
        DEBUG_CHECK_RETURN_EX(unfilteredPoints != nullptr, "Invalid point table");

        // filter points, skip if points are to close
        InplaceArray<RawPoint, 100> points;
        points.pushBack(unfilteredPoints[0]);

        for (uint32_t i = 1; i < numPoints; ++i)
        {
            float td = unfilteredPoints[i].time - points.back().time;
            DEBUG_CHECK_RETURN_EX(td >= 0.0f, "Points not ordered in time order");
            if (td > 0.01f)
            {
                points.pushBack(unfilteredPoints[i]);
            }
        }

        // pack the filtered points
        numPoints = points.size();

        // calc data size
        auto dataSize = sizeof(Header);
        dataSize += sizeof(Point) * (numPoints - 1);

        // add data for points
        const auto initialValueOffset = dataSize;
        dataSize += sizeof(Vector3) * numPoints;

        // initialize data buffer
        m_data = Buffer::CreateEmpty(MainPool(), dataSize, 4);

        //--

        // pack header
        auto* h = (Header*)m_data.data();
        h->loopFlag = looped;
        h->numPoints = numPoints;
        h->minTime = points[0].time;
        h->maxTime = points[numPoints - 1].time;
        h->loopFlag = looped;
        h->invTimeSpan = (h->maxTime > h->minTime) ? (1.0f / (h->maxTime - h->minTime)) : 0.0f;

        // pack points
        auto* valueWritePtr = OffsetPtr<float>(m_data.data(), initialValueOffset);
        for (uint32_t i = 0; i < numPoints; ++i)
        {
            h->points[i].time = points[i].time;
            h->points[i].valueOffset = (char*)valueWritePtr - (char*)m_data.data();
            h->points[i].invTimeToNext = 0.0f;

            const bool hasNext = i < (numPoints - 1);
            if (hasNext)
            {
                const float timeToNext = points[i+1].time - points[i].time;
                h->points[i].invTimeToNext = (timeToNext > 0.0f) ? (1.0f / timeToNext) : 0.0f;
            }

            // pack data
            // TODO: float16 ?
            *valueWritePtr++ = points[i].color.x;
            *valueWritePtr++ = points[i].color.y;
            *valueWritePtr++ = points[i].color.z;
        }
    }
}

void ColorGradient::reset(const Vector3& defaultValue /*=nullptr*/, bool looped /*=false*/)
{
    RawPoint p;
    p.color = defaultValue;

    init(1, &p, looped);
}

uint32_t ColorGradient::points() const
{
    if (const auto* h = header())
        return h->numPoints;
    return 0;
}

bool ColorGradient::looped() const
{
    if (const auto* h = header())
        return h->loopFlag;
    return false;
}

//--

Vector3 ColorGradient::interpolate(float time) const
{
    if (m_data.empty())
        return Vector3::ONE();

    const auto* h = header();
    if (time <= h->minTime)
    {
        const auto* data = OffsetPtr<Vector3>(h, h->points[0].valueOffset);
        return *data;
    }
    else if (time >= h->maxTime)
    {
        const auto* data = OffsetPtr<Vector3>(h, h->points[h->numPoints - 1].valueOffset);
        return *data;
    }

    const auto* searchPoint = h->points + 1; // TODO: arithmetic search
    for (;;)
    {
        if (time <= searchPoint->time)
            break;
        ++searchPoint;
    }

    const auto& pointA = searchPoint[-1];
    const auto& pointB = searchPoint[0];

    const auto* dataA = OffsetPtr<Vector3>(h, pointA.valueOffset);
    const auto* dataB = OffsetPtr<Vector3>(h, pointB.valueOffset);

    const float frac = std::min<float>(1.0f, (time - pointA.time) * pointA.invTimeToNext);
    return dataA->lerp(*dataB, frac);
}

void ColorGradient::unpack(Array<RawPoint>& outPoints) const
{
    if (empty())
        return;

    const auto* h = header();
    outPoints.resize(h->numPoints);
    memset(outPoints.data(), 0, outPoints.dataSize());

    for (uint32_t i = 0; i < h->numPoints; ++i)
    {
        const auto& p = h->points[i];
        auto& op = outPoints[i];

        const Vector3* data = OffsetPtr<const Vector3>(h, p.valueOffset);

        op.time = p.time;
        op.color = *data;
    }
}

//--

END_INFERNO_NAMESPACE()
