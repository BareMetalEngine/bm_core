/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\vector2 #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// A point or vector in 2D space (texcoord mostly)
class BM_CORE_MATH_API Vector2
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(Vector2);

public:
    float x;        //!< X vector component
    float y;        //!< Y vector component

    //--

    INLINE Vector2();
    INLINE Vector2(float inX, float inY);
    INLINE Vector2(const Vector2 &other) = default;
    INLINE Vector2(Vector2 &&other) = default;
    INLINE Vector2& operator=(const Vector2 &other) = default;
    INLINE Vector2& operator=(Vector2 &&other) = default;

    INLINE bool operator==(const Vector2 &other) const;
    INLINE bool operator!=(const Vector2 &other) const;

    INLINE Vector2 operator-() const;
    INLINE Vector2 operator+(const Vector2 &other) const;
    INLINE Vector2 operator+(float value) const;
    INLINE Vector2 operator-(const Vector2 &other) const;
    INLINE Vector2 operator-(float value) const;
    INLINE Vector2 operator*(const Vector2 &other) const;
    INLINE Vector2 operator*(float value) const;
    friend Vector2 operator*(float value, const Vector2 &other);
    INLINE Vector2 operator/(const Vector2 &other) const;
    INLINE Vector2 operator/(float value) const;
    INLINE Vector2 &operator+=(const Vector2 &other);
    INLINE Vector2 &operator+=(float value);
    INLINE Vector2 &operator-=(const Vector2 &other);
    INLINE Vector2 &operator-=(float value);
    INLINE Vector2 &operator*=(const Vector2 &other);
    INLINE Vector2 &operator*=(float value);
    INLINE Vector2 &operator/=(const Vector2 &other);
    INLINE Vector2 &operator/=(float value);
    INLINE float operator|(const Vector2 &other) const;
    INLINE Vector2 operator~() const;

    //--

    //! returns minimal from 2 vector components
    INLINE float minValue() const;

    //! returns maximal from 2 vector components
    INLINE float maxValue() const;

    //! get sum of the components
    INLINE float sum() const;

    //! get product of the components
    INLINE float trace() const;

    //! get vector with absolute components
    INLINE Vector2 abs() const;

    //! round to nearest integer
    INLINE Vector2 round() const;

    //! get fractional part
    INLINE Vector2 frac() const;

    //! truncate fractional part
    INLINE Vector2 trunc() const;

    //! round to lower integer
    INLINE Vector2 floor() const;

    //! round to higher integer
    INLINE Vector2 ceil() const;

    //! square length of vector
    INLINE float squareLength() const;

    //! length of vector
    INLINE float length() const;

    //! 1/length, safe
    INLINE float invLength() const;

    //! check if vector is exactly equal to zero vector
    INLINE bool isZero() const;

    //! check if vector is close to zero vector
    INLINE bool isNearZero(float epsilon = SMALL_EPSILON) const;

    //! check if vector is near enough to another one
    INLINE bool isSimilar(const Vector2 &other, float epsilon = SMALL_EPSILON) const;

    //! normalize vector, does nothing if zero, returns length of the vector before normalization
    INLINE float normalize();

    //! Return normalized vector
    INLINE Vector2 normalized() const;

    //! normalize vector, no checks, returns length
    INLINE float normalizeFast();

    //! Return fast normalized vector, no checks
    INLINE Vector2 normalizedFast() const;

    //! Return perpendicular vector (-y,x)
    INLINE Vector2 prep() const;

    //! Calculate distance to other point
    INLINE float distance(const Vector2 &other) const;

    //! Calculate square distance to other point
    INLINE float squareDistance(const Vector2 &other) const;

    //! get index of largest axis
    INLINE int largestAxis() const;

    //! get index of smallest axis
    INLINE int smallestAxis() const;

    //! get the 1/x 1/y vector, safe
    INLINE Vector2 oneOver() const;

    //! get the 1/x 1/y vector, unsafe
    INLINE Vector2 oneOverFast() const;

    //--

    INLINE Vector2 xx() const;
    INLINE Vector2 yy() const;
    INLINE Vector2 yx() const;
    INLINE Vector3 xyz(float z = 0.0f) const;
    INLINE Vector4 xyzw(float z = 0.0f, float w = 1.0f) const;

    INLINE Vector2& xy();
    INLINE const Vector2& xy() const;

    //--

    static const Vector2& ZERO();
    static const Vector2& ONE();
    static const Vector2& EX();
    static const Vector2& EY();
    static const Vector2& INF();

    //--

    void print(IFormatStream& f) const;

    //--

    // minimum values per component
    INLINE Vector2 min(const Vector2& other) const;

    // minimum value per component
    INLINE Vector2 minScalar(float val) const;

    // maximum values
    INLINE Vector2 max(const Vector2& other) const;

    // maximum values
    INLINE Vector2 maxScalar(float val) const;

    // clamped to range
    INLINE Vector2 clamped(const Vector2& minV, const Vector2& maxV) const;

    // clamped to range
    INLINE Vector2 clampedScalar(float minV, float maxV) const;

    // clamp in place to range
    INLINE void clamp(const Vector2& minV, const Vector2& maxV);

    // clamp in place to range
    INLINE void clampScalar(float minV, float maxV);

    // snap to grid
    INLINE void snap(float grid);

    // snap to grid
    INLINE Vector2 snapped(float grid) const;

    // lerp between this vector and target
    INLINE Vector2 lerp(const Vector2& target, float f) const;

    // lerp between this vector and target, per component weight
    INLINE Vector2 lerpPerComponent(const Vector2& target, const Vector2& f) const;

    //! get a normal component of this vector as projected on given normal vector
    INLINE Vector2 normalPart(const Vector2& normal) const;

    //! get tangent component of this vector as projected on given normal vector
    INLINE Vector2 tangentPart(const Vector2& normal) const;

    //! decompose vector into normal and tangent part, first:normal, second:tangent
    INLINE std::pair<Vector2, Vector2> decompose(const Vector2& normal) const;

    //! dot product with other vector
    INLINE float dot(const Vector2& other) const;

    //! limit vector length, in place
    INLINE void clampLength(float length);

    //! return vector with set length
    INLINE Vector2 clampedLength(float length) const;

    //--

private:
    INLINE Vector2 _xy() const;
};

//--

END_INFERNO_NAMESPACE()
