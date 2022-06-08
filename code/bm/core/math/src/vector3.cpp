/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\vector3 #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//--

RTTI_BEGIN_TYPE_STRUCT(Vector3);
    RTTI_BIND_NATIVE_COMPARE(Vector3);
    RTTI_TYPE_TRAIT().zeroInitializationValid().noConstructor().noDestructor().fastCopyCompare();

    RTTI_PROPERTY(x).editable().scriptVisible();
    RTTI_PROPERTY(y).editable().scriptVisible();
    RTTI_PROPERTY(z).editable().scriptVisible();

    RTTI_NATIVE_CLASS_FUNCTION_EX("Abs", abs);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Trunc", trunc);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Frac", frac);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Round", round);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Ceil", ceil);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Floor", floor);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Sum", sum);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Trace", trace);

    RTTI_NATIVE_CLASS_FUNCTION_EX("IsZero", isZero);
    RTTI_NATIVE_CLASS_FUNCTION_EX("IsNearZero", isNearZero);
    RTTI_NATIVE_CLASS_FUNCTION_EX("IsSimilar", isSimilar);

    RTTI_NATIVE_CLASS_FUNCTION_EX("SquareLength", squareLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Length", length);
    RTTI_NATIVE_CLASS_FUNCTION_EX("InvLength", invLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("OneOver", oneOver);
    RTTI_NATIVE_CLASS_FUNCTION_EX("OneOverFast", oneOverFast);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalize", normalize);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalized", normalized);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Distance", distance);
    RTTI_NATIVE_CLASS_FUNCTION_EX("SquareDistance", squareDistance);

    RTTI_NATIVE_CLASS_FUNCTION_EX("LargestAxis", largestAxis);
    RTTI_NATIVE_CLASS_FUNCTION_EX("SmallestAxis", smallestAxis);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MinValue", minValue);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MaxValue", maxValue);

    RTTI_NATIVE_CLASS_FUNCTION_EX("ToAngles", toRotator);

    RTTI_NATIVE_CLASS_FUNCTION_EX("Min", min);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Max", max);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MinScalar", minScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MaxScalar", maxScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Clamp", clamp);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Clamped", clamped);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampScalar", clampScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampedScalar", clampedScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Dot", dot);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Cross", operator^);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Snap", snap);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Snapped", snapped);
    RTTI_NATIVE_CLASS_FUNCTION_EX("NormalPart", normalPart);
    RTTI_NATIVE_CLASS_FUNCTION_EX("TangentPart", tangentPart);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampLength", clampLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampedLength", clampedLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Lerp", lerp);

RTTI_END_TYPE();

//-----------------------------------------------------------------------------

Angles Vector3::toRotator() const
{
    Angles ret;
    if (!x && !y)
    {
        ret.yaw = 0.0f;
        ret.pitch = (z > 0) ? -90.0f : 90.0f;
        ret.roll = 0.0f;
    }
    else
    {
        ret.yaw =  RAD2DEG * atan2(y, x);
        ret.pitch =  RAD2DEG * atan2(-z, sqrt(x*x + y*y));
        ret.roll = 0.0f;
    }
    return ret;
}

void Vector3::print(IFormatStream& f) const
{
    f.appendf("[{},{},{}]", x, y, z);
}

//--

static Vector3 Vector3_ZERO(0,0,0);
static Vector3 Vector3_ONE(1,1,1);
static Vector3 Vector3_EX(1,0,0);
static Vector3 Vector3_EY(0,1,0);
static Vector3 Vector3_EZ(0,0,1);
static Vector3 Vector3_INF(VERY_LARGE_FLOAT, VERY_LARGE_FLOAT, VERY_LARGE_FLOAT);

const Vector3& Vector3::ZERO() { return Vector3_ZERO; }
const Vector3& Vector3::ONE() { return Vector3_ONE; }
const Vector3& Vector3::EX() { return Vector3_EX; }
const Vector3& Vector3::EY() { return Vector3_EY; }
const Vector3& Vector3::EZ() { return Vector3_EZ; }
const Vector3& Vector3::INF() { return Vector3_INF; }

//--

static Vector3 opNegV3(const Vector3& a) { return -a; }
static Vector3 opAddV3(const Vector3& a, const Vector3& b) { return a + b; }
static Vector3 opSubV3(const Vector3& a, const Vector3& b) { return a - b; }
static Vector3 opMulV3(const Vector3& a, const Vector3& b) { return a * b; }
static Vector3 opDivV3(const Vector3& a, const Vector3& b) { return a / b; }
static float opDotV3(const Vector3& a, const Vector3& b) { return a | b; }
static Vector3 opCrossV3(const Vector3& a, const Vector3& b) { return a ^ b; }

static Vector3 opAsssignAddV3(Vector3& a, const Vector3& b) { return a += b; }
static Vector3 opAsssignSubV3(Vector3& a, const Vector3& b) { return a -= b; }
static Vector3 opAsssignMulV3(Vector3& a, const Vector3& b) { return a *= b; }
static Vector3 opAsssignDivV3(Vector3& a, const Vector3& b) { return a /= b; }

static Vector3 opAsssignAddV3F( Vector3& a, float b) { return a += b; }
static Vector3 opAsssignSubV3F(Vector3& a, float b) { return a -= b; }
static Vector3 opAsssignMulV3F(Vector3& a, float b) { return a *= b; }
static Vector3 opAsssignDivV3F(Vector3& a, float b) { return a /= b; }

static Vector3 opAddV3F(const Vector3& a, float b) { return a + b; }
static Vector3 opSubV3F(const Vector3& a, float b) { return a - b; }
static Vector3 opMulV3F(const Vector3& a, float b) { return a * b; }
static Vector3 opDivV3F(const Vector3& a, float b) { return a / b; }
static Vector3 opMulFV3(float a, const Vector3& b) { return b * a; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opNegV3, "opNegate_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAddV3, "opAdd_ref_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubV3, "opSubtract_ref_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulV3, "opMultiply_ref_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivV3, "opDivide_ref_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDotV3, "opBinaryOr_ref_Vector3_ref_Vector3_float");
RTTI_SCRIPT_GLOBAL_FUNCTION(opCrossV3, "opHat_ref_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddV3, "opAddAssign_out_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubV3, "opSubAssign_out_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulV3, "opMulAssign_out_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivV3, "opDivAssign_out_Vector3_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAddV3F, "opAdd_ref_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubV3F, "opSubtract_ref_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulV3F, "opMultiply_ref_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivV3F, "opDivide_ref_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulFV3, "opMultiply_float_ref_Vector3_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddV3F, "opAddAssign_out_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubV3F, "opSubAssign_out_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulV3F, "opMulAssign_out_Vector3_float_Vector3");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivV3F, "opDivAssign_out_Vector3_float_Vector3");

//--

END_INFERNO_NAMESPACE()
