#include "CameraComponent.h"
#include <glm/ext.hpp>
#include <engine/geometry/Transform.h>
#include <engine/geometry/BBox.h>
#include <engine/util/Logger.h>
#include <imgui/imgui.h>

void Frustum::transform(const glm::mat4& matrix)
{
    for (auto& p : points)
        p = glm::vec3(matrix * glm::vec4(p, 1.0f));
}

BBox Frustum::getBBox() const
{
    BBox bbox;

    for (auto& p : points)
        bbox.unite(p);

    return bbox;
}

CameraComponent::CameraComponent() {}

void CameraComponent::update() { updateViewMatrix(); }

void CameraComponent::onShowInEditor()
{
    const char* mode[]{
            "Perspective",
            "Orthographic"
        };

    int curItem = m_perspective ? 0 : 1;
    ImGui::Combo("Mode", &curItem, mode, 2);
    m_perspective = curItem == 0;

    ImGui::DragFloat("Near Plane", &m_nearZ, 0.01f, 0.01f, 1000.0f);
    ImGui::DragFloat("Far Plane", &m_farZ, 0.01f, 0.0f, 1000.0f);
    ImGui::DragFloat2("Viewport Min", &m_normalizedViewport.min()[0], 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat2("Viewport Max", &m_normalizedViewport.max()[0], 0.01f, 0.0f, 1.0f);

    if (m_perspective) { ImGui::DragFloat("Field of View", &m_fovY, 1.0f, 30.0f, 90.0f); }

    setViewport(m_normalizedViewport.minX() * m_screenWidth, m_normalizedViewport.minY() * m_screenHeight,
                m_normalizedViewport.width() * m_screenWidth, m_normalizedViewport.height() * m_screenHeight);
}

void CameraComponent::setOrthographic(float screenWidth, float screenHeight, float zn, float zf) noexcept
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_nearZ = zn;
    m_farZ = zf;
    m_perspective = false;

    setViewport(0.0f, 0.0f, screenWidth, screenHeight);
}

void CameraComponent::setPerspective(float fovY, float screenWidth, float screenHeight, float zn, float zf) noexcept
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_fovY = fovY;
    m_aspect = screenWidth / screenHeight;
    m_nearZ = zn;
    m_farZ = zf;
    m_perspective = true;

    setViewport(0.0f, 0.0f, screenWidth, screenHeight);
}

void CameraComponent::setViewport(float x, float y, float width, float height) noexcept
{
    m_viewport = Rect(x, y, x + width, y + height);
    m_normalizedViewport = Rect(x / m_screenWidth, y / m_screenHeight,
                                (x + width) / m_screenWidth, (y + height) / m_screenHeight);
    m_aspect = m_viewport.width() / m_viewport.height();

    if (m_perspective)
        m_proj = glm::perspective(m_fovY, m_aspect, m_nearZ, m_farZ);
    else
        m_proj = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, m_nearZ, m_farZ);

    m_projInv = glm::inverse(m_proj);
}

void CameraComponent::resize(float screenWidth, float screenHeight)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    setViewport(m_normalizedViewport.minX() * m_screenWidth, m_normalizedViewport.minY() * m_screenHeight,
                m_normalizedViewport.width() * m_screenWidth, m_normalizedViewport.height() * m_screenHeight);
}

void CameraComponent::updateViewMatrix() noexcept
{
    auto transform = getComponent<Transform>();

    glm::mat3 rotation = glm::toMat3(transform->getRotation());
    glm::vec3 pos = transform->getPosition();

    glm::vec3& right = rotation[0];
    glm::vec3& up = rotation[1];
    glm::vec3& look = rotation[2];

    float x = -glm::dot(pos, right);
    float y = -glm::dot(pos, up);
    float z = -glm::dot(pos, look);

    m_view[0][0] = right.x;
    m_view[1][0] = right.y;
    m_view[2][0] = right.z;
    m_view[3][0] = x;

    m_view[0][1] = up.x;
    m_view[1][1] = up.y;
    m_view[2][1] = up.z;
    m_view[3][1] = y;

    m_view[0][2] = look.x;
    m_view[1][2] = look.y;
    m_view[2][2] = look.z;
    m_view[3][2] = z;

    m_view[0][3] = 0.0f;
    m_view[1][3] = 0.0f;
    m_view[2][3] = 0.0f;
    m_view[3][3] = 1.0f;

    m_viewProj = m_proj * m_view;
    m_viewInv = glm::inverse(m_view);
    m_viewProjInv = m_viewInv * m_projInv;
}

glm::vec3 CameraComponent::screenToWorldPoint(const glm::vec3& p) const noexcept
{
    // Convert to NDC space
    glm::vec4 point(screenToNDC(p), 1.f);

    // Convert to world space
    point = m_viewProjInv * point;
    point /= point.w;

    return glm::vec3(point);
}

glm::vec3 CameraComponent::screenToNDC(const glm::vec3& p) const noexcept
{
    return glm::vec3(
        p.x / m_screenWidth * 2.0f - 1.0f,
        p.y / m_screenHeight * 2.0f - 1.0f,
        (p.z - m_nearZ) / (m_farZ - m_nearZ) * 2.0f - 1.0f);
}

glm::vec3 CameraComponent::worldToScreenPoint(const glm::vec3& worldPoint) const noexcept
{
    glm::vec4 ndc = m_viewProj * glm::vec4(worldPoint, 1.0f);
    ndc /= ndc.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * m_screenWidth,
                     (ndc.y * 0.5f + 0.5f) * m_screenHeight,
                     (ndc.z + 1.f) * (m_farZ - m_nearZ) * 0.5f + m_nearZ);
}

glm::vec3 CameraComponent::screenToViewportPoint(const glm::vec3& p) const noexcept { return glm::vec3(p.x - m_viewport.minX(), p.y - m_viewport.minY(), p.z); }

glm::vec3 CameraComponent::viewportToScreenPoint(const glm::vec3& p) const noexcept { return glm::vec3(p.x + m_viewport.minX(), p.y + m_viewport.minY(), p.z); }

glm::vec3 CameraComponent::viewportToNDC(const glm::vec3& p) const noexcept
{
    return glm::vec3(
        p.x / m_viewport.width() * 2.0f - 1.0f,
        p.y / m_viewport.height() * 2.0f - 1.0f,
        (p.z - m_nearZ) / (m_farZ - m_nearZ) * 2.0f - 1.0f);
}

glm::vec3 CameraComponent::viewportToWorldPoint(const glm::vec3& p) const noexcept
{
    // Convert to NDC space
    glm::vec4 point(viewportToNDC(p), 1.f);

    // Convert to world space
    point = m_viewProjInv * point;
    point /= point.w;

    return glm::vec3(point);
}

glm::vec3 CameraComponent::worldToViewportPoint(const glm::vec3& p) const noexcept
{
    glm::vec4 ndc = m_viewProj * glm::vec4(p, 1.0f);
    ndc /= ndc.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * m_viewport.width(),
                     (0.5f + ndc.y * 0.5f) * m_viewport.height(),
                     (ndc.z + 1.f) * (m_farZ - m_nearZ) * 0.5f + m_nearZ);
}

Ray CameraComponent::screenPointToRay(const glm::vec3& p) const noexcept
{
    // Convert to NDC space
    glm::vec3 ndcP = screenToNDC(p);
    glm::vec4 start(ndcP, 1.f);
    glm::vec4 end(ndcP.x, ndcP.y, 1.0f, 1.f);

    // Convert to world space
    start = m_viewProjInv * start;
    start /= start.w;

    end = m_viewProjInv * end;
    end /= end.w;

    return Ray(glm::vec3(start), glm::normalize(glm::vec3(end - start)));
}

Ray CameraComponent::viewportPointToRay(const glm::vec3& p) const noexcept
{
    // Convert to NDC space
    glm::vec3 ndcP = viewportToNDC(p);
    glm::vec4 start(ndcP, 1.f);
    glm::vec4 end(ndcP.x, ndcP.y, 1.0f, 1.f);

    // Convert to world space
    start = m_viewProjInv * start;
    start /= start.w;

    end = m_viewProjInv * end;
    end /= end.w;

    return Ray(glm::vec3(start), glm::normalize(glm::vec3(end - start)));
}

Frustum CameraComponent::getFrustum() const { return getFrustum(m_nearZ, m_farZ); }

Frustum CameraComponent::getFrustum(float nearZ, float farZ) const
{
    Frustum frustum;

    float thHFOV = tanf(math::toRadians(getHorizontalFOV() * 0.5f));
    float thVFOV = tanf(math::toRadians(m_fovY * 0.5f));

    // Compute the frustum in view space
    float xNear = nearZ * thHFOV;
    float yNear = nearZ * thVFOV;
    float xFar = farZ * thHFOV;
    float yFar = farZ * thVFOV;

    // Transform to world space
    frustum.nearBottomLeft = glm::vec3(m_viewInv * glm::vec4(-xNear, -yNear, nearZ, 1.0f));
    frustum.nearBottomRight = glm::vec3(m_viewInv * glm::vec4(xNear, -yNear, nearZ, 1.0f));
    frustum.nearTopLeft = glm::vec3(m_viewInv * glm::vec4(-xNear, yNear, nearZ, 1.0f));
    frustum.nearTopRight = glm::vec3(m_viewInv * glm::vec4(xNear, yNear, nearZ, 1.0f));

    frustum.farBottomLeft = glm::vec3(m_viewInv * glm::vec4(-xFar, -yFar, farZ, 1.0f));
    frustum.farBottomRight = glm::vec3(m_viewInv * glm::vec4(xFar, -yFar, farZ, 1.0f));
    frustum.farTopLeft = glm::vec3(m_viewInv * glm::vec4(-xFar, yFar, farZ, 1.0f));
    frustum.farTopRight = glm::vec3(m_viewInv * glm::vec4(xFar, yFar, farZ, 1.0f));

    /*
    glm::vec4 nearBottomLeft  = m_viewProjInv * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
    glm::vec4 nearBottomRight = m_viewProjInv * glm::vec4(1.0f, -1.0f, -1.0f, 1.0f);
    glm::vec4 nearTopLeft	  = m_viewProjInv * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
    glm::vec4 nearTopRight	  = m_viewProjInv * glm::vec4(1.0f, 1.0f, -1.0f, 1.0f);

    glm::vec4 farBottomLeft	  = m_viewProjInv * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);
    glm::vec4 farBottomRight  = m_viewProjInv * glm::vec4(1.0f, -1.0f, 1.0f, 1.0f);
    glm::vec4 farTopLeft	  = m_viewProjInv * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 farTopRight	  = m_viewProjInv * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    frustum.nearBottomLeft  = glm::vec3(nearBottomLeft / nearBottomLeft.w);
    frustum.nearBottomRight = glm::vec3(nearBottomRight / nearBottomRight.w);
    frustum.nearTopLeft     = glm::vec3(nearTopLeft / nearTopLeft.w);
    frustum.nearTopRight    = glm::vec3(nearTopRight / nearTopRight.w);
                                        
    frustum.farBottomLeft   = glm::vec3(farBottomLeft / farBottomLeft.w);
    frustum.farBottomRight	= glm::vec3(farBottomRight / farBottomRight.w);
    frustum.farTopLeft		= glm::vec3(farTopLeft / farTopLeft.w);
    frustum.farTopRight		= glm::vec3(farTopRight / farTopRight.w);
    */

    return frustum;
}
