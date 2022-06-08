/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\absolute #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//--

RTTI_BEGIN_TYPE_STRUCT(ExactPosition);
    RTTI_BIND_NATIVE_COMPARE(ExactPosition);
    RTTI_PROPERTY(x).editable().scriptVisible();
    RTTI_PROPERTY(y).editable().scriptVisible();
    RTTI_PROPERTY(z).editable().scriptVisible();

    RTTI_SCRIPT_CLASS_FUNCTION_EX("Distance", distance);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("DistanceFast", distanceFast);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ExactDistance", exactDistance);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("SquareDistance", squareDistance);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("SquareDistanceFast", squareDistanceFast);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ExactSquareDistance", exactSquareDistance);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Snap", snap);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Snapped", snapped);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToVector", approx);

RTTI_END_TYPE();

//--

static ExactPosition ROOT_ABSPOS(0.0f, 0.0f, 0.0f);

void ExactPosition::print(IFormatStream& f) const
{
    f.appendf("{x={<1.4>}, y={<1.4>}, z={<1.4>}}", x, y, z);
}

const ExactPosition& ExactPosition::ZERO()
{
    return ROOT_ABSPOS;
}

//--

static ExactPosition opNegP(const ExactPosition& a) { return -a; }

static ExactPosition opAddPP(const ExactPosition& a, const ExactPosition& b) { return a + b; }
static ExactPosition opSubPP(const ExactPosition& a, const ExactPosition& b) { return a - b; }

static ExactPosition opAsssignAddPP(ExactPosition& a, const ExactPosition& b) { return a += b; }
static ExactPosition opAsssignSubPP(ExactPosition& a, const ExactPosition& b) { return a -= b; }

static ExactPosition opAddPV(const ExactPosition& a, const Vector3& b) { return a + b; }
static ExactPosition opSubPV(const ExactPosition& a, const Vector3& b) { return a - b; }
static ExactPosition opMulPV(const ExactPosition& a, const Vector3& b) { return a * b; }
static ExactPosition opMulPF(const ExactPosition& a, float b) { return a * b; }

static ExactPosition opAsssignAddPV(ExactPosition& a, const Vector3& b) { return a += b; }
static ExactPosition opAsssignSubPV(ExactPosition& a, const Vector3& b) { return a -= b; }
static ExactPosition opAsssignMulPV(ExactPosition& a, const Vector3& b) { return a *= b; }
static ExactPosition opAsssignMulPF(ExactPosition& a, float b) { return a *= b; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opNegP, "opNegate_ref_ExactPosition_ExactPosition");

RTTI_SCRIPT_GLOBAL_FUNCTION(opAddPP, "opAdd_ref_ExactPosition_ref_ExactPosition_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubPP, "opSubtract_ref_ExactPosition_ref_ExactPosition_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddPP, "opAddAssign_out_ExactPosition_ref_ExactPosition_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubPP, "opSubAssign_out_ExactPosition_ref_ExactPosition_ExactPosition");

RTTI_SCRIPT_GLOBAL_FUNCTION(opAddPV, "opAdd_ref_ExactPosition_ref_Vector3_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubPV, "opSubtract_ref_ExactPosition_ref_Vector3_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulPV, "opMultiply_ref_ExactPosition_ref_Vector3_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulPF, "opMultiply_ref_ExactPosition_float_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddPV, "opAddAssign_out_ExactPosition_ref_Vector3_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubPV, "opSubAssign_out_ExactPosition_ref_Vector3_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulPV, "opMulAssign_out_ExactPosition_ref_Vector3_ExactPosition");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulPF, "opMulAssign_out_ExactPosition_float_ExactPosition");

//--

END_INFERNO_NAMESPACE()
