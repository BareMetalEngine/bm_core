/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

class BM_CORE_MATH_API Angles
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(Angles);

public:
    float roll = 0.0f; // X, CW
    float pitch = 0.0f; // Y, CCW
    float yaw = 0.0f; // Z, CW

    //---

    INLINE Angles() = default;
    INLINE Angles(float inPitch, float inYaw, float inRoll);
    INLINE Angles(const Angles &other) = default;
    INLINE Angles(Angles&& other) = default;
    INLINE Angles& operator=(const Angles &other) = default;
    INLINE Angles& operator=(Angles&& other) = default;

    INLINE bool operator==(const Angles &other) const;
    INLINE bool operator!=(const Angles &other) const;

    INLINE Angles operator-() const;
    INLINE Angles operator+(const Angles &other) const;
    INLINE Angles &operator+=(const Angles &other);
    INLINE Angles operator-(const Angles &other) const;
    INLINE Angles &operator-=(const Angles &other);
    INLINE Angles &operator*=(float value);
    INLINE Angles operator*(float value) const;
    INLINE Angles &operator/=(float value);
    INLINE Angles operator/(float value) const;

    //--

    //! normalize rotation (keep values within -360 to 360 range)
    void normalize();

    //! get normalized rotation
    Angles normalized() const;

    //! get "distance" to other angles (NOTE: this is very basic)
    Angles distance(const Angles &other) const;

    //! are all angles zero
    INLINE bool isZero() const;

    //! are all angles really close to zero ?
    INLINE bool isNearZero(float eps = SMALL_EPSILON) const;

    //! get angles with absolute values
    INLINE Angles abs() const;

    //! get maximum angle value
    INLINE float maxValue() const;

    //! get minimum angle value
    INLINE float minValue() const;

    //! get sum of all angles
    INLINE float sum() const;

    //! get axis with smallest rotation
    INLINE uint8_t smallestAxis() const;

    //! get axis with largest rotation
    INLINE uint8_t largestAxis() const;

    //--

    //! Get the forward direction
    Vector3 forward() const;

    //! Get the side direction
    Vector3 side() const;

    //! Get the up direction
    Vector3 up() const;

    //--

    //! get new rotation angles that moves us from current angles to target ones by maximum step "degrees"
    void approach(const Angles &target, float degrees);

    //--

    //! get rotation basis vectors
    void angleVectors(Vector3& forward, Vector3& right, Vector3& up) const;

    //! get some of the rotation basis vectors
    void someAngleVectors(Vector3 *forward, Vector3 *right, Vector3 *up) const;

    //--

    //! convert to matrix holding this rotation
    Matrix toMatrix() const;

    //! convert to transposed matrix that represents inverted rotation
    Matrix toMatrixTransposed() const;

    //! convert to 3x3 matrix holding this rotation
    Matrix33 toMatrix33() const;

    //! convert to 3x3 matrix holding this rotation
    Matrix33 toMatrixTransposed33() const;

    //! convert to quaternion holding this rotation
    Quat toQuat() const;

    //! convert to quaternion holding this rotation
    Quat toQuatInverted() const;

    //--

    static const Angles& ZERO();
    static const Angles& X90_CW();
    static const Angles& X90_CCW();
    static const Angles& Y90_CW();
    static const Angles& Y90_CCW();
    static const Angles& Z90_CW();
    static const Angles& Z90_CCW();

    //--

    void print(IFormatStream& f) const;

    //--

    // snap to grid
    INLINE void snap(float grid);

    // snap to grid
    INLINE Angles snapped(float grid) const;

    // compute "dot product" between rotations - dot of the foward vectors
    INLINE float dot(const Angles& other) const;

    // compute "dot product" between rotations - dot of the foward vectors
    INLINE float dotAxis(const Vector3& other) const;

    //--

    // minimum values per component
    INLINE Angles min(const Angles& other) const;

    // minimum value per component
    INLINE Angles minScalar(float val) const;

    // maximum values
    INLINE Angles max(const Angles& other) const;

    // maximum values
    INLINE Angles maxScalar(float val) const;

    // clamped to range
    INLINE Angles clamped(const Angles& minV, const Angles& maxV) const;

    // clamped to range
    INLINE Angles clampedScalar(float minV, float maxV) const;

    // clamp in place to range
    INLINE void clamp(const Angles& minV, const Angles& maxV);

    // clamp in place to range
    INLINE void clampScalar(float minV, float maxV);

    // lerp between this vector and target
    Angles lerp(const Angles& target, float f) const;

    // lerp between this vector and target
    Angles lerpNormalized(const Angles& target, float f) const;

    //--

};

END_INFERNO_NAMESPACE()
