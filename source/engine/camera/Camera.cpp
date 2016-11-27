#include "Camera.h"
#include <glm/ext.hpp>

Camera::Camera()
    : m_pos(0, 0, 0), m_right(1, 0, 0), m_up(0, 1, 0), m_look(0, 0, 1) {}

void Camera::setOrthographic(float screenWidth, float screenHeight, float zn, float zf)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_nearZ = zn;
    m_farZ = zf;
    m_perspective = false;

    setViewport(0.0f, 0.0f, screenWidth, screenHeight);
}

void Camera::setPerspective(float fovY, float screenWidth, float screenHeight, float zn, float zf)
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

void Camera::setViewport(float x, float y, float width, float height)
{
    if (x + width <= 0.0f || y + height <= 0.0f)
        return;

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

void Camera::resize(float screenWidth, float screenHeight)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    setViewport(m_normalizedViewport.minX() * m_screenWidth, m_normalizedViewport.minY() * m_screenHeight,
                m_normalizedViewport.width() * m_screenWidth, m_normalizedViewport.height() * m_screenHeight);
}

void Camera::lookAt(const glm::vec3& target, const glm::vec3& worldUp)
{
    m_look = glm::normalize(target - m_pos);
    m_right = glm::normalize(glm::cross(worldUp, m_look));
    m_up = glm::cross(m_look, m_right);
}

void Camera::pitch(float angle)
{
    // Rotate up and look vector about the right vector.
    glm::mat4 rotation = glm::rotate(angle, m_right);

    m_look = glm::vec3(rotation * glm::vec4(m_look, 0));
    m_up = glm::vec3(rotation * glm::vec4(m_up, 0));
}

void Camera::rotateY(float angle)
{
    // Rotate the basis vectors about the world y-axis.
    m_right = glm::rotateY(m_right, angle);
    m_up = glm::rotateY(m_up, angle);
    m_look = glm::rotateY(m_look, angle);
}

void Camera::roll(float angle)
{
    // Rotate up and right vector about the look vector.
    glm::mat4 rotation = glm::rotate(angle, m_look);

    m_right = glm::vec3(rotation * glm::vec4(m_right, 0));
    m_up = glm::vec3(rotation * glm::vec4(m_up, 0));
}

void Camera::updateViewMatrix()
{
    // Orthonormalize
    m_look = normalize(m_look);
    m_up = normalize(cross(m_look, m_right));
    m_right = cross(m_up, m_look);

    float x = -glm::dot(m_pos, m_right);
    float y = -glm::dot(m_pos, m_up);
    float z = -glm::dot(m_pos, m_look);

    m_view[0][0] = m_right.x;
    m_view[1][0] = m_right.y;
    m_view[2][0] = m_right.z;
    m_view[3][0] = x;

    m_view[0][1] = m_up.x;
    m_view[1][1] = m_up.y;
    m_view[2][1] = m_up.z;
    m_view[3][1] = y;

    m_view[0][2] = m_look.x;
    m_view[1][2] = m_look.y;
    m_view[2][2] = m_look.z;
    m_view[3][2] = z;

    m_view[0][3] = 0.0f;
    m_view[1][3] = 0.0f;
    m_view[2][3] = 0.0f;
    m_view[3][3] = 1.0f;

    m_viewProj = m_proj * m_view;
    m_viewInv = glm::inverse(m_view);
    m_viewProjInv = m_viewInv * m_projInv;
}

glm::vec3 Camera::screenToWorldPoint(const glm::vec3& p) const
{
    // Convert to NDC space
    glm::vec4 point(screenToNDC(p), 1.f);

    // Convert to world space
    point = m_viewProjInv * point;
    point /= point.w;

    return glm::vec3(point);
}

glm::vec3 Camera::screenToNDC(const glm::vec3& p) const
{
    return glm::vec3(
        p.x / m_screenWidth * 2.0f - 1.0f,
        p.y / m_screenHeight * 2.0f - 1.0f,
        (p.z - m_nearZ) / (m_farZ - m_nearZ) * 2.0f - 1.0f);
}

glm::vec3 Camera::worldToScreenPoint(const glm::vec3& worldPoint) const
{
    glm::vec4 ndc = m_viewProj * glm::vec4(worldPoint, 1.0f);
    ndc /= ndc.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * m_screenWidth,
                     (ndc.y * 0.5f + 0.5f) * m_screenHeight,
                     (ndc.z + 1.f) * (m_farZ - m_nearZ) * 0.5f + m_nearZ);
}

glm::vec3 Camera::screenToViewportPoint(const glm::vec3& p) const { return glm::vec3(p.x - m_viewport.minX(), p.y - m_viewport.minY(), p.z); }

glm::vec3 Camera::viewportToScreenPoint(const glm::vec3& p) const { return glm::vec3(p.x + m_viewport.minX(), p.y + m_viewport.minY(), p.z); }

glm::vec3 Camera::viewportToNDC(const glm::vec3& p) const
{
    return glm::vec3(
        p.x / m_viewport.width() * 2.0f - 1.0f,
        p.y / m_viewport.height() * 2.0f - 1.0f,
        (p.z - m_nearZ) / (m_farZ - m_nearZ) * 2.0f - 1.0f);
}

glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& p) const
{
    // Convert to NDC space
    glm::vec4 point(viewportToNDC(p), 1.f);

    // Convert to world space
    point = m_viewProjInv * point;
    point /= point.w;

    return glm::vec3(point);
}

glm::vec3 Camera::worldToViewportPoint(const glm::vec3& p) const
{
    glm::vec4 ndc = m_viewProj * glm::vec4(p, 1.0f);
    ndc /= ndc.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * m_viewport.width(),
                     (0.5f + ndc.y * 0.5f) * m_viewport.height(),
                     (ndc.z + 1.f) * (m_farZ - m_nearZ) * 0.5f + m_nearZ);
}

Ray Camera::screenPointToRay(const glm::vec3& p) const
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

Ray Camera::viewportPointToRay(const glm::vec3& p) const
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
