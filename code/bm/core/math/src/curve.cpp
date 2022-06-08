/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "curve.h"

BEGIN_INFERNO_NAMESPACE()

//---

RTTI_BEGIN_TYPE_STRUCT(Curve);
    RTTI_TYPE_TRAIT().zeroInitializationValid();
    RTTI_PROPERTY(m_data);
RTTI_END_TYPE();

//---

Curve::Curve() = default;
Curve::Curve(const Curve& other) = default;
Curve& Curve::operator=(const Curve& other) = default;
Curve::~Curve() = default;
Curve::Curve(Curve && other) = default;
Curve& Curve::operator=(Curve && other) = default;

Curve::Curve(float defaultvalue, bool looped)
{
    reset(defaultvalue, looped);
}

Curve::Curve(uint32_t numPoints, const RawPoint* points, bool looped)
{
    init(numPoints, points, looped);
}

void Curve::clear()
{
    m_data.reset();
}

void Curve::init(uint32_t numPoints, const RawPoint* unfilteredPoints, bool looped)
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
        const auto initialValueOffset = dataSize;

        // add data for points
        for (uint32_t i = 0; i < numPoints; ++i)
        {
            switch (points[i].type)
            {
            case CurveSegmentType::Constant:
            case CurveSegmentType::Linear:
                dataSize += sizeof(float);
                break;

            case CurveSegmentType::Hermite:
            case CurveSegmentType::Bezier:
                dataSize += sizeof(float) * 5; // value, cp0 time and value, cp1 time and value,
                break;
            }
        }

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
            h->points[i].type = (int)points[i].type;
            h->points[i].handles = (int)points[i].handles;
            h->points[i].valueOffset = (char*)valueWritePtr - (char*)m_data.data();
            h->points[i].invTimeToNext = 0.0f;

            const bool hasNext = i < (numPoints - 1);
            if (hasNext)
            {
                const float timeToNext = points[i+1].time - points[i].time;
                h->points[i].invTimeToNext = (timeToNext > 0.0f) ? (1.0f / timeToNext) : 0.0f;
            }

            // demote last bezier 
            else if (points[i].type == CurveSegmentType::Bezier)
            {
                h->points[i].type = (int)CurveSegmentType::Linear;
            }

            // pack data
            switch ((CurveSegmentType)h->points[i].type)
            {
                case CurveSegmentType::Constant:
                case CurveSegmentType::Linear:
                case CurveSegmentType::Hermite:
                {
                    *valueWritePtr++ = points[i].value;
                    break;
                }

                case CurveSegmentType::Bezier:
                {
                    // make absolute
                    float ct0 = points[i].c1.x;
                    float cv0 = points[i].c1.y;
                    float ct1 = (points[i+1].c0.x + points[i+1].time) - points[i].time;
                    float cv1 = (points[i+1].c0.y + points[i+1].value) - points[i].value;

                    // normalize time range
                    float timeSpan = points[i + 1].time - points[i].time;
                    ct0 /= timeSpan;
                    ct1 /= timeSpan;

                    // correct so bezier does not form a loop
                    float scale0 = 1.0f;
                    float scale1 = 1.0f;
                    if (Curve::CorrectBezzierHandles(0.0f, ct0, ct1, 1.0f, scale0, scale1))
                    {
                        ct0 *= scale0;
                        cv0 *= scale0;
                        ct1 *= scale1;
                        cv1 *= scale1;
                    }

                    // pack data
                    //  value
                    //  control point 0: normalized time offset
                    //  control point 0: value relative to base value
                    //  control point 1: normalized time offset
                    //  control point 1: value relative to base value

                    *valueWritePtr++ = points[i].value;
                    *valueWritePtr++ = ct0;
                    *valueWritePtr++ = cv0;
                    *valueWritePtr++ = ct1;
                    *valueWritePtr++ = cv1;
                    break;
                }
            }
        }
    }
}

void Curve::reset(float defaultValue /*=nullptr*/, bool looped /*=false*/)
{
    RawPoint p;
    p.value = defaultValue;

    init(1, &p, looped);
}

uint32_t Curve::points() const
{
    if (const auto* h = header())
        return h->numPoints;
    return 0;
}

bool Curve::looped() const
{
    if (const auto* h = header())
        return h->loopFlag;
    return false;
}

//--

float Curve::interpolate(float time) const
{
    if (m_data.empty())
        return 0.0f;

    const auto* h = header();
    if (time <= h->minTime)
    {
        const auto* data = OffsetPtr<float>(h, h->points[0].valueOffset);
        return data[0];
    }
    else if (time >= h->maxTime)
    {
        const auto* data = OffsetPtr<float>(h, h->points[h->numPoints - 1].valueOffset);
        return data[0];
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

    const auto* dataA = OffsetPtr<float>(h, pointA.valueOffset);
    const auto* dataB = OffsetPtr<float>(h, pointB.valueOffset);

    const float frac = std::min<float>(1.0f, (time - pointA.time) * pointA.invTimeToNext);

    switch ((CurveSegmentType)pointA.type)
    {
        case CurveSegmentType::Constant:
            break;

        case CurveSegmentType::Hermite:
        case CurveSegmentType::Linear:
        {
            return dataA[0] + (dataB[0] - dataA[0]) * frac;
        }

        case CurveSegmentType::Bezier:
        {
            float v0 = dataA[0];
            float ct0 = dataA[1];
            float cv0 = dataA[2];
            float ct1 = dataA[3];
            float cv1 = dataA[4];
            float v1 = dataB[0];

            // find the intersection point
            float roots[8];
            roots[0] = 0.0f;
            Curve::FindZero(frac, 0.0f, ct0, ct1, 1.0f, roots);

            // blend values
            return Curve::InterpolateBezier(v0, v0 + cv0, v0 + cv1, v1, roots[0]);
        }
    }

    // fall back to const
    return dataA[0];
}

void Curve::unpack(Array<RawPoint>& outPoints) const
{
    if (empty())
        return;

    const auto* h = header();
    outPoints.resize(h->numPoints);
    memset(outPoints.data(), 0, outPoints.dataSize());

    for (uint32_t i=0; i<h->numPoints; ++i)
    {
        const auto& p = h->points[i];
        auto& op = outPoints[i];

        op.time = p.time;
        op.type = (CurveSegmentType)p.type;
        op.handles = (CurveBezierHandleType)p.handles;

        const float* data = OffsetPtr<const float>(h, p.valueOffset);
        op.value = data[0];

        if (op.type == CurveSegmentType::Bezier)
        {
            if (i < (h->numPoints - 1))
            {
                const auto& np = h->points[i + 1];
                
                const float timeSpan = np.time - p.time;
                if (timeSpan > 0.0f)
                {
                    // tangent on this point
                    op.c1.x = data[1] * timeSpan; // unpack from normalized range
                    op.c1.y = data[2]; // make control point value relative to current value

                    // unpack data for next point
                    const float* ndata = OffsetPtr<const float>(h, np.valueOffset);

                    // calculate delta point
                    auto& nop = outPoints[i + 1];
                    nop.c0.x = ((data[3] * timeSpan) + p.time) - np.time;
                    nop.c0.y = (data[4] + op.value) - ndata[0];  // make control point value relative to next value
                }
            }
        }
    }
}

//---

/// Copied from Blender
/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2009 Blender Foundation, Joshua Leung
 * All rights reserved.
*/

bool Curve::CorrectBezzierHandles(float v1, float v2, float v3, float v4, float& outScale1, float& outScale2)
{
    // Calculate handle deltas
    float h1 = v1 - v2;
    float h2 = v4 - v3;

    // Calculate distances:
    // len  = Span of time between keyframes.
    // len1 = Length of handle of start key.
    // len2 = Length of handle of end key.
    float len = v4 - v1;
    float len1 = std::fabs(h1);
    float len2 = std::fabs(h2);

    // Assume no corrections
    outScale1 = 1.0f;
    outScale2 = 1.0f; 

    // If the handles have no length, no need to do any corrections
    if ((len1 + len2) == 0.0f)
        return false;

    // To prevent looping or rewinding, handles cannot exceed the adjacent key-frames time position
    bool corrected = false;
    if (len1 > len)
    {
        outScale1 = len / len1;
        corrected = true;
    }

    if (len2 > len)
    {
        outScale2 = len / len2;
        corrected = true;
    }

    return corrected;

/*      v2[0] = (v1[0] - fac * h1[0]);
        v2[1] = (v1[1] - fac * h1[1]);
        // 2
        v3[0] = (v4[0] - fac * h2[0]);
        v3[1] = (v4[1] - fac * h2[1]);
    */
}

#define UNLIKELY(x) x
#define SMALL -1.0e-10

double sqrt3d(double d)
{
    if (UNLIKELY(d == 0.0)) {
        return 0.0;
    }
    else if (UNLIKELY(d < 0.0)) {
        return -exp(log(-d) / 3.0);
    }
    else {
        return exp(log(d) / 3.0);
    }
}

// Find roots of cubic equation (c0 x^3 + c1 x^2 + c2 x + c3)
int Curve::SolveCubic(double c0, double c1, double c2, double c3, float* o)
{
    double a, b, c, p, q, d, t, phi;
    int nr = 0;

    if (c3 != 0.0) {
        a = c2 / c3;
        b = c1 / c3;
        c = c0 / c3;
        a = a / 3;

        p = b / 3 - a * a;
        q = (2 * a * a * a - a * b + c) / 2;
        d = q * q + p * p * p;

        if (d > 0.0) {
            t = sqrt(d);
            o[0] = (float)(sqrt3d(-q + t) + sqrt3d(-q - t) - a);

            if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) {
                return 1;
            }
            return 0;
        }

        if (d == 0.0) {
            t = sqrt3d(-q);
            o[0] = (float)(2 * t - a);

            if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) {
                nr++;
            }
            o[nr] = (float)(-t - a);

            if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) {
                return nr + 1;
            }
            return nr;
        }

        phi = acos(-q / sqrt(-(p * p * p)));
        t = sqrt(-p);
        p = cos(phi / 3);
        q = sqrt(3 - 3 * p * p);
        o[0] = (float)(2 * t * p - a);

        if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) {
            nr++;
        }
        o[nr] = (float)(-t * (p + q) - a);

        if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) {
            nr++;
        }
        o[nr] = (float)(-t * (p - q) - a);

        if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) {
            return nr + 1;
        }
        return nr;
    }
    a = c2;
    b = c1;
    c = c0;

    if (a != 0.0) {
        /* Discriminant */
        p = b * b - 4 * a * c;

        if (p > 0) {
            p = sqrt(p);
            o[0] = (float)((-b - p) / (2 * a));

            if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) {
                nr++;
            }
            o[nr] = (float)((-b + p) / (2 * a));

            if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) {
                return nr + 1;
            }
            return nr;
        }

        if (p == 0) {
            o[0] = (float)(-b / (2 * a));
            if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) {
                return 1;
            }
        }

        return 0;
    }

    if (b != 0.0) {
        o[0] = (float)(-c / b);

        if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) {
            return 1;
        }
        return 0;
    }

    if (c == 0.0) {
        o[0] = 0.0;
        return 1;
    }

    return 0;
}

int Curve::FindZero(float x, float q0, float q1, float q2, float q3, float* o)
{
    const double c0 = q0 - x;
    const double c1 = 3.0f * (q1 - q0);
    const double c2 = 3.0f * (q0 - 2.0f * q1 + q2);
    const double c3 = q3 - q0 + 3.0f * (q1 - q2);

    return SolveCubic(c0, c1, c2, c3, o);
}

float Curve::InterpolateBezier(float f1, float f2, float f3, float f4, float t)
{
    float it = 1.0f - t;
    return (f1 * it * it * it) + (f2 * it * it * t * 3.0f) + (f3 * it * t * t * 3.0f) + (f4 * t * t * t);

    /*float c0 = f1;
    float c1 = 3.0f * (f2 - f1);
    float c2 = 3.0f * (f1 - 2.0f * f2 + f3);
    float c3 = f4 - f1 + 3.0f * (f2 - f3);

    return c0 + (t * c1) + (t * t * c2) + (t * t * t * c3);*/
}

//--

END_INFERNO_NAMESPACE()
