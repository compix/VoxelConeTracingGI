#pragma once
#include <glm/glm.hpp>
#include <engine/geometry/Rect.h>
#include <engine/geometry/Ray.h>

/**
* Default assumption for screen coordinates is (0, 0) in lower left corner, (screenWidth, screenHeight) in upper right corner.
* The viewport is described in screen coordinates.
*/
class Camera
{
    friend class CameraSystem;
public:
    Camera();

    void setOrthographic(float screenWidth, float screenHeight, float zn, float zf);
    void setPerspective(float fovY, float screenWidth, float screenHeight, float zn, float zf);
    void setViewport(float x, float y, float width, float height);
    void resize(float screenWidth, float screenHeight);

    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0.f, 1.f, 0.f));

    const glm::mat4& viewProj() const { return m_viewProj; }

    void updateViewMatrix();

    void strafe(float d) { m_pos += m_right * d; }

    void walk(float d) { m_pos += m_look * d; }

    void pitch(float angle);
    void rotateY(float angle);
    void roll(float angle);

    const glm::vec3& getPosition() const { return m_pos; }

    void setPosition(const glm::vec3& p) { m_pos = p; }

    void setPosition(float x, float y, float z) { m_pos = glm::vec3(x, y, z); }

    void translate(const glm::vec3& v) { m_pos += v; }

    void zoom(float val) { walk(val); }

    float getNearClipPlane() const { return m_nearZ; }

    float getFarClipPlane() const { return m_farZ; }

    const glm::vec3& getRight() const { return m_right; }

    const glm::vec3& getUp() const { return m_up; }

    glm::vec3 getForward() const { return m_look; }

    const glm::mat4& view() const { return m_view; }

    const glm::mat4& proj() const { return m_proj; }

    const glm::mat4& viewInverse() const { return m_viewInv; }

    const glm::mat4& projInverse() const { return m_projInv; }

    const glm::mat4& viewProjInv() const { return m_viewProjInv; }

    float getScreenWidth() const { return m_screenWidth; }

    float getScreenHeight() const { return m_screenHeight; }

    const Rect& getViewport() const { return m_viewport; }

    void setProj(glm::mat4& proj) { m_proj = proj; }

    void setView(glm::mat4& view) { m_view = view; }

    /**
    * Flips y coordinate in screen space.
    */
    glm::vec3 flipY(const glm::vec3& p) const { return glm::vec3(p.x, m_screenHeight - p.y, p.z); }

    /**
    * Screen point has 3 components where x, y are in screen space and z describes the position in world units relative to the camera.
    */
    glm::vec3 screenToWorldPoint(const glm::vec3& p) const;

    /**
    * Screen point has 3 components where x, y are in screen space and z describes the position in world units relative to the camera.
    */
    glm::vec3 screenToNDC(const glm::vec3& p) const;
    glm::vec3 worldToScreenPoint(const glm::vec3& worldPoint) const;
    glm::vec3 screenToViewportPoint(const glm::vec3& p) const;
    glm::vec3 viewportToScreenPoint(const glm::vec3& p) const;
    glm::vec3 viewportToNDC(const glm::vec3& p) const;
    glm::vec3 viewportToWorldPoint(const glm::vec3& p) const;
    glm::vec3 worldToViewportPoint(const glm::vec3& p) const;
    Ray screenPointToRay(const glm::vec3& p) const;
    Ray viewportPointToRay(const glm::vec3& p) const;
protected:
    bool m_perspective{true};

    glm::vec3 m_pos;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_look;

    float m_nearZ{0.f};
    float m_farZ{1.f};
    float m_aspect{1.f};
    float m_fovY{1.f};

    float m_screenWidth{0.f};
    float m_screenHeight{0.f};

    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::mat4 m_viewProj;
    glm::mat4 m_viewInv;
    glm::mat4 m_projInv;
    glm::mat4 m_viewProjInv;

    Rect m_viewport;
    Rect m_normalizedViewport;
};
