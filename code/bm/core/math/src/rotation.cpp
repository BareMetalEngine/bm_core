/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\rotation #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

RTTI_BEGIN_TYPE_STRUCT(Angles);
    RTTI_BIND_NATIVE_COMPARE(Angles);
    RTTI_TYPE_TRAIT().zeroInitializationValid().noConstructor().noDestructor().fastCopyCompare();
    RTTI_PROPERTY(pitch).editable("Pitch around Y axis").range(-360.0f, 360.0f).widgetDrag(true).scriptVisible();
    RTTI_PROPERTY(yaw).editable("Yaw around Z axis").range(-360.0f, 360.0f).widgetDrag(true).scriptVisible();
    RTTI_PROPERTY(roll).editable("Roll around X axis").range(-360.0f, 360.0f).widgetDrag(true).scriptVisible();

    RTTI_NATIVE_CLASS_FUNCTION_EX("ForwardVector", forward);
    RTTI_NATIVE_CLASS_FUNCTION_EX("SideVector", side);
    RTTI_NATIVE_CLASS_FUNCTION_EX("UpVector", up);
    RTTI_NATIVE_CLASS_FUNCTION_EX("IsZero", isZero);
    RTTI_NATIVE_CLASS_FUNCTION_EX("IsNearZero", isNearZero);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Distance", distance);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalize", normalize);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalized", normalized);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Approach", approach);
    RTTI_NATIVE_CLASS_FUNCTION_EX("AngleVectors", angleVectors);

    RTTI_NATIVE_CLASS_FUNCTION_EX("ToMatrix", toMatrix);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ToMatrixTransposed", toMatrixTransposed);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ToMatrix33", toMatrix33);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ToMatrixTransposed33", toMatrixTransposed33);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ToQuat", toQuat);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ToQuatInverted", toQuatInverted);

    RTTI_NATIVE_CLASS_FUNCTION_EX("Abs", abs);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Dot", dot);
    RTTI_NATIVE_CLASS_FUNCTION_EX("DotAxis", dotAxis);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Snap", snap);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Snapped", snapped);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Min", min);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Max", max);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MinScalar", minScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MaxScalar", maxScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Clamp", clamp);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Clamped", clamped);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampScalar", clampScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampedScalar", clampedScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Lerp", lerp);
    RTTI_NATIVE_CLASS_FUNCTION_EX("LerpNormalized", lerpNormalized);

RTTI_END_TYPE();

//-----------------------------------------------------------------------------

void Angles::normalize()
{
    pitch = AngleNormalize(pitch);
    yaw = AngleNormalize(yaw);
    roll = AngleNormalize(roll);
}

Angles Angles::normalized() const
{
    return Angles(AngleNormalize(pitch), AngleNormalize(yaw), AngleNormalize(roll));
}

Angles Angles::distance(const Angles &other) const
{
    float localPitch = AngleDistance(pitch, other.pitch);
    float localYaw = AngleDistance(yaw, other.yaw);
    float localRoll = AngleDistance(roll, other.roll);
    return Angles(localPitch, localYaw, localRoll);
}

void Angles::approach(const Angles &b, float move)
{
    pitch = AngleReach(pitch, b.pitch, move);
    yaw = AngleReach(yaw, b.yaw, move);
    roll = AngleReach(roll, b.roll, move);
}

/*Angles Angles::Rand(float minAngle, float maxAngle, bool withRoll)
{
    return Angles(RandRange(minAngle, maxAngle),
            RandRange(minAngle, maxAngle),
        withRoll ? RandRange(minAngle, maxAngle) : 0.0f);
}*/

Angles Angles::lerp(const Angles& target, float f) const
{
    return Angles(
        Lerp<float>(pitch, target.pitch, f),
        Lerp<float>(yaw, target.yaw, f),
        Lerp<float>(roll, target.roll, f));
}

Angles Angles::lerpNormalized(const Angles& target, float f) const
{
    return Angles(
        AngleNormalize(Lerp<float>(pitch, target.pitch, f)),
        AngleNormalize(Lerp<float>(yaw, target.yaw, f)),
        AngleNormalize(Lerp<float>(roll, target.roll, f)));
}

void Angles::angleVectors(Vector3& forward, Vector3& right, Vector3& up) const
{
    return someAngleVectors(&forward, &right, &up);
}

void Angles::someAngleVectors(Vector3 *forward, Vector3 *right, Vector3 *up) const
{
    // Rotation order: X(Roll) Y(Pitch) Z(Yaw)
    // All rotations are CW
    // All equations derived using Mathematica

    float yawRad = DEG2RAD * yaw;
    float pitchRad = DEG2RAD * pitch;
    float rollRad = DEG2RAD * roll;

    float sinYaw = sin(yawRad);
    float cosYaw = cos(yawRad);
    float sinPitch = sin(pitchRad);
    float cosPitch = cos(pitchRad);
    float sinRoll = sin(rollRad);
    float cosRoll = cos(rollRad);

    if (forward)
        *forward = Vector3(cosPitch*cosYaw, cosPitch*sinYaw, -sinPitch);

    if (right)
        *right = Vector3(sinRoll*sinPitch*cosYaw - cosRoll*sinYaw, cosRoll*cosYaw + sinRoll*sinPitch*sinYaw, sinRoll*cosPitch);

    if (up)
        *up = Vector3(cosRoll*sinPitch*cosYaw + sinRoll*sinYaw, cosRoll*sinPitch*sinYaw - sinRoll*cosYaw, cosRoll*cosPitch);
}


Vector3 Angles::forward() const
{
    float yawRad = DEG2RAD * yaw;
    float pitchRad = DEG2RAD * pitch;

    float sinYaw = sin(yawRad);
    float cosYaw = cos(yawRad);
    float sinPitch = sin(pitchRad);
    float cosPitch = cos(pitchRad);

    return Vector3( cosPitch*cosYaw, cosPitch*sinYaw, -sinPitch );
}

Vector3 Angles::side() const
{
    float yawRad = DEG2RAD * yaw;
    float pitchRad = DEG2RAD * pitch;
    float rollRad = DEG2RAD * roll;

    float sinYaw = sin(yawRad);
    float cosYaw = cos(yawRad);
    float sinPitch = sin(pitchRad);
    float cosPitch = cos(pitchRad);
    float sinRoll = sin(rollRad);
    float cosRoll = cos(rollRad);

    return Vector3(sinRoll*sinPitch*cosYaw - cosRoll*sinYaw, cosRoll*cosYaw + sinRoll*sinPitch*sinYaw, sinRoll*cosPitch);
}

Vector3 Angles::up() const
{
    float yawRad = DEG2RAD * yaw;
    float pitchRad = DEG2RAD * pitch;
    float rollRad = DEG2RAD * roll;

    float sinYaw = sin(yawRad);
    float cosYaw = cos(yawRad);
    float sinPitch = sin(pitchRad);
    float cosPitch = cos(pitchRad);
    float sinRoll = sin(rollRad);
    float cosRoll = cos(rollRad);

    return Vector3(cosRoll*sinPitch*cosYaw + sinRoll*sinYaw, cosRoll*sinPitch*sinYaw - sinRoll*cosYaw, cosRoll*cosPitch);
}

Matrix Angles::toMatrix() const
{
    Vector3 x,y,z;
    angleVectors(x,y,z);

    return Matrix(x.x, y.x, z.x,
                    x.y, y.y, z.y,
                    x.z, y.z, z.z);
}

Matrix Angles::toMatrixTransposed() const
{
    return toMatrix().transposed();
}

Matrix33 Angles::toMatrix33() const
{
    Vector3 x,y,z;
    angleVectors(x,y,z);

    return Matrix33(x.x, y.x, z.x,
                    x.y, y.y, z.y,
                    x.z, y.z, z.z);
}

Matrix33 Angles::toMatrixTransposed33() const
{
    return toMatrix33().transposed();
}

Quat Angles::toQuat() const
{
    Quat ret;

    // Calculate half angles
    float halfR = DEG2RAD * roll * 0.5f;
    float halfY = DEG2RAD * yaw * 0.5f;
    float halfP = DEG2RAD * pitch * 0.5f;

    // Calculate rotation components
    float sr = sin( halfR );
    float cr = cos( halfR );
    float sy = sin( halfY );
    float cy = cos( halfY );
    float sp = sin( halfP );
    float cp = cos( halfP );

    // Assemble quaternion
    ret.x = sr*cp*cy-cr*sp*sy; // X
    ret.y = cr*sp*cy+sr*cp*sy; // Y
    ret.z = cr*cp*sy-sr*sp*cy; // Z
    ret.w = cr*cp*cy+sr*sp*sy; // W

    // Make sure output quaternion is normalized
    return ret;
}

Quat Angles::toQuatInverted() const
{
    return toQuat().inverted();
}

static Angles Angles_ZERO(0,0,0);
static Angles Angles_X90_CW(0.0f, 0.0f, 90.0f);
static Angles Angles_X90_CCW(0.0f, 0.0f, -90.0f);
static Angles Angles_Y90_CW(90.0f, 0.0f, 0.0f);
static Angles Angles_Y90_CCW(-90.0f, 0.0f, 0.0f);
static Angles Angles_Z90_CW(0.0f, 90.0f, 0.0f);
static Angles Angles_Z90_CCW(0.0f, -90.0f, 0.0f);

const Angles& Angles::ZERO() { return Angles_ZERO; }
const Angles& Angles::X90_CW() { return Angles_X90_CW; };
const Angles& Angles::X90_CCW() { return Angles_X90_CCW; };
const Angles& Angles::Y90_CW() { return Angles_Y90_CW; };
const Angles& Angles::Y90_CCW() { return Angles_Y90_CCW; };
const Angles& Angles::Z90_CW() { return Angles_Z90_CW; };
const Angles& Angles::Z90_CCW() { return Angles_Z90_CCW; };

//--

void Angles::print(IFormatStream& f) const
{
    f.append("[");

    bool hasSomething = false;

    if (pitch != 0.0f)
    {
        f.appendf("pitch={}", pitch);
        hasSomething = true;
    }

    if (yaw != 0.0f)
    {
        if (hasSomething)
            f << ", ";
        f.appendf("yaw={}", yaw);
        hasSomething = true;
    }

    if (roll != 0.0f)
    {
        if (hasSomething)
            f << ", ";
        f.appendf("roll={}", roll);
        hasSomething = true;
    }

    if (!hasSomething)
        f << "identity";

    f.append("]");
}

//--

static Angles opNegA(const Angles& a) { return -a; }
static Angles opAddA(const Angles& a, const Angles& b) { return a + b; }
static Angles opSubA(const Angles& a, const Angles& b) { return a - b; }
static Angles opMulAF(const Angles& a, float b) { return a * b; }
static Angles opDivAF(const Angles& a, float b) { return a / b; }
static float opDotA(const Angles& a, const Angles& b) { return a.dot(b); }

static Angles opAsssignAddA(Angles& a, const Angles& b) { return a += b; }
static Angles opAsssignSubA(Angles& a, const Angles& b) { return a -= b; }
static Angles opAsssignMulAF(Angles& a, float b) { return a *= b; }
static Angles opAsssignDivAF(Angles& a, float b) { return a /= b; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opNegA, "opNegate_ref_Angles_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAddA, "opAdd_ref_Angles_ref_Angles_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubA, "opSubtract_ref_Angles_ref_Angles_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulAF, "opMultiply_ref_Angles_float_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivAF, "opDivide_ref_Angles_float_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDotA, "opBinaryOr_ref_Angles_ref_Angles_float");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddA, "opAddAssign_out_Angles_ref_Angles_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubA, "opSubAssign_out_Angles_ref_Angles_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulAF, "opMulAssign_out_Angles_float_Angles");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivAF, "opDivAssign_out_Angles_float_Angles");

//--

END_INFERNO_NAMESPACE()
