/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: camera #]
***/

#include "build.h"
#include "camera.h"

BEGIN_INFERNO_NAMESPACE()

///--

// the conversion matrices between engine space and rendering space
//Matrix CameraSetup::CameraToView(Vector4(0, 1, 0, 0), Vector4(0, 0, -1, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1));
Matrix CameraSetup::CameraToView(Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1));
Matrix CameraSetup::ViewToCamera = CameraToView.inverted();

RTTI_BEGIN_TYPE_STRUCT(CameraSetup);
RTTI_PROPERTY(position).scriptVisible();
RTTI_PROPERTY(rotation).scriptVisible();
RTTI_PROPERTY(fov).scriptVisible();
RTTI_PROPERTY(zoom).scriptVisible();
RTTI_PROPERTY(aspect).scriptVisible();
RTTI_PROPERTY(nearPlane).scriptVisible();
RTTI_PROPERTY(farPlane).scriptVisible();
RTTI_PROPERTY(subPixelOffsetX).scriptVisible();
RTTI_PROPERTY(subPixelOffsetY).scriptVisible();
RTTI_PROPERTY(discontinuityIndex).scriptVisible();
RTTI_END_TYPE();

CameraSetup::CameraSetup()
    : position(Vector3::ZERO())
    , rotation(Quat::IDENTITY())
    , fov(90.0f)
    , zoom(1.0f)
    , aspect(1.0f)
    , nearPlane(0.1f)
    , farPlane(1000.0f)
    , subPixelOffsetX(0.0f)
    , subPixelOffsetY(0.0f)
	, discontinuityIndex(0)
{}

void CameraSetup::calcWorldToCamera(Matrix& outWorldToCamera) const
{
    outWorldToCamera = rotation.inverted().toMatrix();
    outWorldToCamera.translation(-outWorldToCamera.transformVector(position));
}

void CameraSetup::calcCameraToWorld(Matrix &outCameraToWorld) const
{
    outCameraToWorld = rotation.toMatrix();
    outCameraToWorld .translation(position);
}

void CameraSetup::calcViewToScreen(Matrix& outViewToScreen) const
{
    if (perspective())
        outViewToScreen = Matrix::BuildPerspectiveFOV(fov, aspect, nearPlane, farPlane, subPixelOffsetX, subPixelOffsetY, reverseProjection);
    else
        outViewToScreen = Matrix::BuildOrtho(zoom * aspect, zoom, nearPlane, farPlane);
}

void CameraSetup::calcWorldToScreen(Matrix& outWorldToScreen) const
{
    Matrix worldToCamera;
    calcWorldToCamera(worldToCamera);

    Matrix viewToScreen;
    calcViewToScreen(viewToScreen);

    outWorldToScreen = worldToCamera * (CameraSetup::CameraToView * viewToScreen);
}

void CameraSetup::calcAxes(Vector3* outForward, Vector3* outRight, Vector3* outUp) const
{
    if (outForward || outRight || outUp)
    {
        auto t = rotation.toMatrix();

        if (outForward)
            *outForward = t.transformVector(Vector3::EX());

        if (outRight)
            *outRight = t.transformVector(Vector3::EY());

        if (outUp)
            *outUp = t.transformVector(Vector3::EZ());
    }
}

///--

Camera::Camera()
    : m_frameForward(Vector3::EX())
    , m_frameUp(Vector3::EZ())
    , m_frameRight(Vector3::EY())
{
}

Camera::Camera(const Camera& other) = default;
Camera::Camera(Camera&& other) = default;
Camera& Camera::operator=(const Camera& other) = default;
Camera& Camera::operator=(Camera&& other) = default;

Camera::Camera(const CameraSetup& setup)
    : m_setup(setup)
{
    updateMatrices();
}

void Camera::setup(const CameraSetup& setupParams)
{
    m_setup = setupParams;
    updateMatrices();
}

void Camera::updateMatrices()
{
    // get data
    m_setup.calcViewToScreen(m_viewToScreen);
    m_setup.calcWorldToCamera(m_worldToCamera);
    m_setup.calcCameraToWorld(m_cameraToWorld);
    m_setup.calcAxes(&m_frameForward, &m_frameRight, &m_frameUp);

    // calc extra matrices
    m_screenToView = m_viewToScreen.inverted();
    m_cameraToScreen = CameraSetup::CameraToView * m_viewToScreen;
    m_screenToCamera = m_screenToView * CameraSetup::ViewToCamera;

    // calc final matrix
    m_worldToScreen = m_worldToCamera * m_cameraToScreen;
    m_screenToWorld = m_screenToCamera * m_cameraToWorld;
}

float Camera::calcScreenSpaceScalingFactor(const Vector3 &pos, uint32_t width, uint32_t height) const
{
    // transform point from world space to screen space
    auto screenSpacePoint = worldToScreen().transformVector4(Vector4(pos, 1.0f));

    // calculate pixel size in world space at given depth
    auto pixelDeltaX = (2.0f / (float)width) * screenSpacePoint.w;
    auto pixelDeltaY = (2.0f / (float)height) * screenSpacePoint.w;
    return std::max<float>(pixelDeltaX, pixelDeltaY);
}

bool Camera::calcWorldSpaceRay(const Vector3& screenPosition, Vector3& outRayStart, Vector3& outRayDirection) const
{
    // calculate screen space point
    Vector4 screenSpacePoint;
    screenSpacePoint.x = (screenPosition.x * 2.0f) - 1.0f;
    screenSpacePoint.y = (screenPosition.y * 2.0f) - 1.0f;
    screenSpacePoint.z = 0.0f;
    screenSpacePoint.w = 1.0f;

    // transform to point on the near plane to world space
    auto startPoint = screenToWorld().transformVector4(screenSpacePoint);
    startPoint.project();

    // setup the end point at the far plane
    screenSpacePoint.z = 1.0f;
    auto endPoint = screenToWorld().transformVector4(screenSpacePoint);
    endPoint.project();

    // Use camera origin as ray origin
    outRayStart = startPoint.xyz();
    outRayDirection = (endPoint - startPoint).xyz();
    outRayDirection.normalize();
    return true;
}

bool Camera::projectWorldToScreen(const Vector3 &worldPosition, Vector3 &outScreenPosition) const
{
    Vector4 worldPos(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
    auto screenPos = m_worldToScreen.transformVector4(worldPos);

    // we don't want to divide by 0, the world is not yet ready for Naked Singularity
    auto w = screenPos.w;
    if (w >= -SMALL_EPSILON && w <= SMALL_EPSILON)
        return false;

    // translate to uniform coordinates
    auto projectedScreenPos = screenPos / screenPos.w;

    // check if pos2D.X and pos2D.Y are now from [-1; 1] range
    auto x = projectedScreenPos.x;
    auto y = projectedScreenPos.y;
    auto z = projectedScreenPos.z;
    if (/*x > 1.0f || x < -1.0f || y > 1.0f || y < -1.0f ||*/ z >= 1.0f)
        return false;

    // calculate the linear distance from the camera plane
    outScreenPosition.x = (x + 1.0f) / 2.0f;
    outScreenPosition.y = 1.0f - ((y + 1.0f) / 2.0f);
    outScreenPosition.z = directionForward() | (worldPosition - position());
    return true;
}

bool Camera::projectScreenToWorld(Vector3& outWorldPosition, const Vector3& screenPosition) const
{
    Vector4 screenSpacePoint;
    screenSpacePoint.x = (screenPosition.x * 2.0f) - 1.0f;
    screenSpacePoint.y = (screenPosition.y * 2.0f) - 1.0f;
    screenSpacePoint.z = 0.0f;
    screenSpacePoint.w = 1.0f;

    Vector4 worldPosition = screenToWorld().transformVector4(screenSpacePoint);
    auto w = worldPosition.w;
    if (w >= -SMALL_EPSILON && w <= SMALL_EPSILON)
        return false;

    worldPosition.projectFast();
    outWorldPosition = worldPosition.xyz();

    return true;
}

void Camera::calcFrustumCorners(float zPlane, Vector3* outCorners) const
{
    outCorners[0] = screenToWorld().transformVector4(Vector4(1.0f, -1.0f, zPlane, 1.0f)).projected();
    outCorners[1] = screenToWorld().transformVector4(Vector4(1.0f, 1.0f, zPlane, 1.0f)).projected();
    outCorners[2] = screenToWorld().transformVector4(Vector4(-1.0f, 1.0f, zPlane, 1.0f)).projected();
    outCorners[3] = screenToWorld().transformVector4(Vector4(-1.0f, -1.0f, zPlane, 1.0f)).projected();
}

// screenZ = (worldZ * m22 + m23) / worldZ;
// screenZ = m22 + m23 / worldZ;
// screenZ - m22 = m23 / worldZ;
// (screenZ - m22) / m23 = 1 / worldZ;
// worldZ = m23 / (screenZ - m22);

float Camera::linearZToProjectedZ(float linearZ) const
{
    if (linearZ <= m_setup.nearPlane)
        return 0.0f;
    else if (linearZ >= m_setup.farPlane)
        return 1.0f;

    float m22 = viewToScreen().m[2][2];
    float m23 = viewToScreen().m[2][3];
    return (linearZ * m22 + m23) / linearZ;
}

float Camera::projectedZToLinearZ(float projectedZ) const
{
    if (projectedZ <= 0.0f)
        return m_setup.nearPlane;
    else if (projectedZ >= 1.0f)
        return m_setup.farPlane;

    float m22 = viewToScreen().m[2][2];
    float m23 = viewToScreen().m[2][3];
    return m23 / (projectedZ - m22);
}

Vector4 Camera::linearZVector() const
{
    Vector4 ret;
    ret.x = viewToScreen().m[0][0];
    ret.y = viewToScreen().m[1][1];
    ret.z = viewToScreen().m[2][2];
    ret.w = viewToScreen().m[2][3];
    return ret;
}

//---

namespace helper
{

    // Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix
    // Gil Gribb, Klaus Hartmann
    static void CalculateFrustumPlaneFromMatrix(uint32_t index, const Matrix& matrix, SIMDQuad& outPlane, SIMDQuad& outMask)
    {
        float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

        switch (index)
        {
        case VisibilityFrustum::PLANE_NEAR:
        {
            x = matrix.m[3][0];
            y = matrix.m[3][1];
            z = matrix.m[3][2];
            w = matrix.m[3][3];
            break;
        }
        case VisibilityFrustum::PLANE_RIGHT:
        {
            x = matrix.m[3][0] - matrix.m[0][0];
            y = matrix.m[3][1] - matrix.m[0][1];
            z = matrix.m[3][2] - matrix.m[0][2];
            w = matrix.m[3][3] - matrix.m[0][3];
            break;
        }
        case VisibilityFrustum::PLANE_LEFT:
        {
            x = matrix.m[3][0] + matrix.m[0][0];
            y = matrix.m[3][1] + matrix.m[0][1];
            z = matrix.m[3][2] + matrix.m[0][2];
            w = matrix.m[3][3] + matrix.m[0][3];
            break;
        }
        case VisibilityFrustum::PLANE_BOTTOM:
        {
            x = matrix.m[3][0] + matrix.m[1][0];
            y = matrix.m[3][1] + matrix.m[1][1];
            z = matrix.m[3][2] + matrix.m[1][2];
            w = matrix.m[3][3] + matrix.m[1][3];
            break;
        }
        case VisibilityFrustum::PLANE_TOP:
        {
            x = matrix.m[3][0] - matrix.m[1][0];
            y = matrix.m[3][1] - matrix.m[1][1];
            z = matrix.m[3][2] - matrix.m[1][2];
            w = matrix.m[3][3] - matrix.m[1][3];
            break;
        }
        case VisibilityFrustum::PLANE_FAR:
        {
            x = matrix.m[3][0] + matrix.m[2][0];
            y = matrix.m[3][1] + matrix.m[2][1];
            z = matrix.m[3][2] + matrix.m[2][2];
            w = matrix.m[3][3] + matrix.m[2][3];
            break;
        }
        }

        auto len = std::sqrt(x * x + y * y + z * z);
        outPlane = SIMDQuad(x / len, y / len, z / len, w / len);
        outMask = outPlane.cmpL(SIMDQuad());
    }

    static void CalculateFrustumPlaneFromPoints(const Vector3& a, const Vector3& b, const Vector3& c, SIMDQuad& outPlane, SIMDQuad& outMask)
    {
        auto n = -TriangleNormal(a, b, c);
        outPlane = SIMDQuad(n.x, n.y, n.z, -(n | a));
        outMask = outPlane.cmpL(SIMDQuad());
    }

    static INLINE bool BoxBehindPlane(const SIMDQuad& boxMin, const SIMDQuad& boxMax, const SIMDQuad& planeMask, const SIMDQuad& planeValues, uint8_t& outIntersects)
    {
        auto vmin = Select(boxMin, boxMax, planeMask);
        auto vmax = Select(boxMax, boxMin, planeMask);
        auto dmin = vmin.dot4(planeValues); // distance of closest point to plane
        auto dmax = vmax.dot4(planeValues); // distance of furthest point to plane

        // check intersection
        auto xorRes = dmin ^ dmax;
        if (xorRes.isNegative())
        {
            outIntersects = VisibilityFrustum::TEST_INTERSECTS;
            return false;
        }

        return dmax.isNegative();
    }

} // helper

void VisibilityFrustum::setup(const Camera& sourceCamera)
{
    if (sourceCamera.isPerspective())
        setupPerspective(sourceCamera.position(), sourceCamera.worldToScreen());
    else
        setupOrtho(sourceCamera.position(), sourceCamera.screenToWorld());

#ifdef BUILD_DEBUG
    auto centerV = sourceCamera.position() + sourceCamera.directionForward();
    auto center = SIMDQuad(centerV.x, centerV.y, centerV.z, 1.0f);

    for (uint32_t i = 0; i < 6; ++i)
    {
        auto distToCenter = planes[i].dot4(center)[0];
        ASSERT(distToCenter > 0.0f);
    }
#endif
}

void VisibilityFrustum::setupPerspective(const Vector3& position, const Matrix& worldToScreen)
{
    origin = SIMDQuad(position.x, position.y, position.z, 1.0f);

    for (uint8_t i = 0; i < MAX_PLANES; ++i)
        helper::CalculateFrustumPlaneFromMatrix(i, worldToScreen, planes[i], planeMask[i]);
}

void VisibilityFrustum::setupOrtho(const Vector3& position, const Matrix& screenToWorld)
{
    Vector3 positions[8];
    positions[0] = Vector3(-1.0f, 1.0f, 0.0f);
    positions[1] = Vector3(1.0f, 1.0f, 0.0f);
    positions[2] = Vector3(1.0f, -1.0f, 0.0f);
    positions[3] = Vector3(-1.0f, -1.0f, 0.0f);
    positions[4] = Vector3(-1.0f, 1.0f, 1.0f);
    positions[5] = Vector3(1.0f, 1.0f, 1.0f);
    positions[6] = Vector3(1.0f, -1.0f, 1.0f);
    positions[7] = Vector3(-1.0f, -1.0f, 1.0f);

    for (uint8_t i = 0; i < ARRAY_COUNT(positions); ++i)
        positions[i] = screenToWorld.transformPoint(positions[i]);

    helper::CalculateFrustumPlaneFromPoints(positions[0], positions[1], positions[2], planes[PLANE_NEAR], planeMask[PLANE_NEAR]);
    helper::CalculateFrustumPlaneFromPoints(positions[7], positions[6], positions[5], planes[PLANE_FAR], planeMask[PLANE_FAR]);
    helper::CalculateFrustumPlaneFromPoints(positions[1], positions[0], positions[5], planes[PLANE_LEFT], planeMask[PLANE_LEFT]);
    helper::CalculateFrustumPlaneFromPoints(positions[2], positions[1], positions[6], planes[PLANE_TOP], planeMask[PLANE_TOP]);
    helper::CalculateFrustumPlaneFromPoints(positions[3], positions[2], positions[7], planes[PLANE_RIGHT], planeMask[PLANE_RIGHT]);
    helper::CalculateFrustumPlaneFromPoints(positions[0], positions[3], positions[4], planes[PLANE_BOTTOM], planeMask[PLANE_BOTTOM]);
}

uint8_t VisibilityFrustum::testBox(const VisibilityBox& box) const
{
    uint8_t result = TEST_INSIDE;

    if (helper::BoxBehindPlane(box.min, box.max, planeMask[0], planes[0], result)
        || helper::BoxBehindPlane(box.min, box.max, planeMask[1], planes[1], result)
        || helper::BoxBehindPlane(box.min, box.max, planeMask[2], planes[2], result)
        || helper::BoxBehindPlane(box.min, box.max, planeMask[3], planes[3], result)
        || helper::BoxBehindPlane(box.min, box.max, planeMask[4], planes[4], result)
        || helper::BoxBehindPlane(box.min, box.max, planeMask[5], planes[5], result)
        ) {
        return TEST_OUTSIDE;
    }

    return result;
}

void VisibilityFrustum::extractPlanes(Vector4* outPlanes) const
{
    outPlanes[0] = planes[0].toVector4();
    outPlanes[1] = planes[1].toVector4();
    outPlanes[2] = planes[2].toVector4();
    outPlanes[3] = planes[3].toVector4();
    outPlanes[4] = planes[4].toVector4();
    outPlanes[5] = planes[5].toVector4();
}

//---

void VisibilityBox::setup(const Box& box)
{
    min = SIMDQuad(box.min.x, box.min.y, box.min.z, 1.0f);
    max = SIMDQuad(box.max.x, box.max.y, box.max.z, 1.0f);
}

///--

END_INFERNO_NAMESPACE()
