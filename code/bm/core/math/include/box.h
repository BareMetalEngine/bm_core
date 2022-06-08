/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\box #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// axis aligned bounding box
class BM_CORE_MATH_API Box
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(Box);

public:
    Vector3 min;
    Vector3 max;

    //--

    INLINE Box();
    INLINE Box(const Vector3 &amin, const Vector3 &amax);
    INLINE Box(const Vector3 &center, float radius);
    INLINE Box(const Vector3 &start, const Vector3 &end, const Vector3 &extents);
    INLINE Box(const Vector3 &start, const Vector3 &end, const Box &extents);
    INLINE Box(const Vector3& start, const Vector3& end, float extents);
    INLINE Box(const Box &other) = default;
    INLINE Box(Box&& other) = default;
    INLINE Box& operator=(const Box &other) = default;
    INLINE Box& operator=(Box&& other) = default;

    INLINE bool operator==(const Box& other) const;
    INLINE bool operator!=(const Box& other) const;

    INLINE Box operator+(const Vector3& other) const;
    INLINE Box operator-(const Vector3& other) const;
    INLINE Box operator+(float margin) const;
    INLINE Box operator-(float margin) const;
    INLINE Box operator*(float scale) const;
    INLINE Box operator/(float scale) const;

    INLINE Box& operator+=(const Vector3& other);
    INLINE Box& operator-=(const Vector3& other);
    INLINE Box& operator+=(float margin);
    INLINE Box& operator-=(float margin);
    INLINE Box& operator*=(float scale);
    INLINE Box& operator/=(float scale);

    //--

    //! clear bounding box, makes it empty
    INLINE void clear();

    //! is bounding box empty ?
    INLINE bool empty() const;

    //! test if given point is inside this box
    INLINE bool containsPoint(const Vector3 &point) const;

    //! test if given point is inside this box
    INLINE bool containsPoint2D(const Vector2 &point) const;

    //! test if given box is inside this box
    INLINE bool containsBox(const Box &box) const;

    //! test if given box is inside this box
    INLINE bool containsBox2D(const Box &box) const;

    //! test if box touches given box
    INLINE bool touchesBox(const Box &other) const;

    //! test if box touches given box
    INLINE bool touchesBox2D(const Box &other) const;

    //! get bounding box corner (valid vertex range 0-7)
    INLINE Vector3 corner(int index) const;

    //! get all of the box corners (8 of them)
    INLINE void corners(Vector3* outCorners) const;

    //! extrude bounding box by given margin
    INLINE void extrude(float margin);

    //! extrude bounding box by given margin
    INLINE Box extruded(float margin) const;

    //! add point to box
    INLINE void updateWithPoint(const Vector3 &point);

    //! add box to box
    INLINE void updateWithBox(const Box &box);

    //! get box volume
    INLINE float volume() const;

    //! get box size (max - min)
    INLINE Vector3 size() const;

    //! calculate box extents (half size)
    INLINE Vector3 extents() const;

    //! calculate center of the box
    INLINE Vector3 center() const;

    //! calculate bounds of the box - it's just the box in this case
    INLINE const Box& bounds() const;

    //--

    //! Zero box (0,0,0) - (0,0,0)
    static const Box& ZERO();

    //! Invalid box (MAX_INF) - (MIN_INF)
    static const Box& EMPTY();

    //! Unit box (0,0,0)-(1,1,1)
    static const Box& UNIT();

    //--

    // intersect with a ray, returns distance to point of entry
    bool intersect(const Vector3& origin, const Vector3& direction, float maxLength = VERY_LARGE_FLOAT, float* outEnterDistFromOrigin = nullptr, Vector3 * outEntryPoint = nullptr, Vector3 * outEntryNormal = nullptr) const;

    //--

    // calculate distance to the bbox face, returns false if we are inside, can return the closest point as well
    bool distance(const Vector3 & position, float& outDistance, Vector3 * outClosestPoint) const;

    //--

private:
    INLINE bool _intersect_NoPoint(const Vector3 & origin, const Vector3 & direction, float maxLength) const { return intersect(origin, direction, maxLength); }
    INLINE bool _intersect_WithPoint(const Vector3 & origin, const Vector3 & direction, float maxLength, float& outEnterDistFromOrigin, Vector3 & outEntryPoint, Vector3 & outEntryNormal) const { return intersect(origin, direction, maxLength, &outEnterDistFromOrigin, &outEntryPoint, &outEntryNormal); }
};

//--

END_INFERNO_NAMESPACE()
