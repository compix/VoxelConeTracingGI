#pragma once
#include <glm/glm.hpp>
#include <engine/geometry/Rect.h>
#include <engine/geometry/Ray.h>
#include <engine/ecs/EntityManager.h>
#include <engine/geometry/Transform.h>

class BBox;

struct Frustum
{
    Frustum() { }

    union
    {
        glm::vec3 points[8];

        struct
        {
            glm::vec3 nearBottomLeft;
            glm::vec3 nearBottomRight;
            glm::vec3 nearTopLeft;
            glm::vec3 nearTopRight;

            glm::vec3 farBottomLeft;
            glm::vec3 farBottomRight;
            glm::vec3 farTopLeft;
            glm::vec3 farTopRight;
        };
    };

    void transform(const glm::mat4& matrix);
    BBox getBBox() const;
};

/**
* Default assumption for screen coordinates is (0, 0) in lower left corner, (screenWidth, screenHeight) in upper right corner.
* The viewport is described in screen coordinates.
*/
class CameraComponent : public Component
{
    friend class CameraSystem;
public:
    CameraComponent();

    void update() override;
    void onShowInEditor() override;

    std::string getName() const override { return "Camera"; }

    void setOrthographic(float screenWidth, float screenHeight, float zn, float zf) noexcept;
    void setPerspective(float fovY, float screenWidth, float screenHeight, float zn, float zf) noexcept;
    void setViewport(float x, float y, float width, float height) noexcept;
    void resize(float screenWidth, float screenHeight);

    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0.f, 1.f, 0.f)) const noexcept { getComponent<Transform>()->lookAt(target, worldUp); }

    const glm::mat4& viewProj() const noexcept { return m_viewProj; }

    void updateViewMatrix() noexcept;

    void strafe(float d) const noexcept { getComponent<Transform>()->strafe(d); }

    void walk(float d) const noexcept { getComponent<Transform>()->walk(d); }

    void pitch(float angle) const noexcept { getComponent<Transform>()->pitch(angle); }

    void rotateY(float angle) const noexcept { getComponent<Transform>()->rotateY(angle); }

    void roll(float angle) const noexcept { getComponent<Transform>()->roll(angle); }

    glm::vec3 getPosition() const noexcept { return getComponent<Transform>()->getPosition(); }

    void setPosition(const glm::vec3& p) const noexcept { getComponent<Transform>()->setPosition(p); }

    void setPosition(float x, float y, float z) const noexcept { getComponent<Transform>()->setPosition(glm::vec3(x, y, z)); }

    void translate(const glm::vec3& v) const noexcept { getComponent<Transform>()->move(v); }

    void zoom(float val) const noexcept { walk(val); }

    float getNearClipPlane() const { return m_nearZ; }

    float getFarClipPlane() const { return m_farZ; }

    glm::vec3 getRight() const noexcept { return getComponent<Transform>()->getRight(); }

    glm::vec3 getUp() const noexcept { return getComponent<Transform>()->getUp(); }

    glm::vec3 getForward() const noexcept { return getComponent<Transform>()->getForward(); }

    const glm::mat4& view() const noexcept { return m_view; }

    const glm::mat4& proj() const noexcept { return m_proj; }

    const glm::mat4& viewInverse() const noexcept { return m_viewInv; }

    const glm::mat4& projInverse() const noexcept { return m_projInv; }

    const glm::mat4& viewProjInv() const noexcept { return m_viewProjInv; }

    float getScreenWidth() const noexcept { return m_screenWidth; }

    float getScreenHeight() const noexcept { return m_screenHeight; }

    const Rect& getViewport() const noexcept { return m_viewport; }

    void setProj(glm::mat4& proj) { m_proj = proj; }

    void setView(glm::mat4& view) { m_view = view; }

    /**
    * Flips y coordinate in screen space.
    */
    glm::vec3 flipY(const glm::vec3& p) const { return glm::vec3(p.x, m_screenHeight - p.y, p.z); }

    /**
    * Screen point has 3 components where x, y are in screen space and z describes the position in world units relative to the camera.
    */
    glm::vec3 screenToWorldPoint(const glm::vec3& p) const noexcept;

    /**
    * Screen point has 3 components where x, y are in screen space and z describes the position in world units relative to the camera.
    */
    glm::vec3 screenToNDC(const glm::vec3& p) const noexcept;
    glm::vec3 worldToScreenPoint(const glm::vec3& worldPoint) const noexcept;
    glm::vec3 screenToViewportPoint(const glm::vec3& p) const noexcept;
    glm::vec3 viewportToScreenPoint(const glm::vec3& p) const noexcept;
    glm::vec3 viewportToNDC(const glm::vec3& p) const noexcept;
    glm::vec3 viewportToWorldPoint(const glm::vec3& p) const noexcept;
    glm::vec3 worldToViewportPoint(const glm::vec3& p) const noexcept;
    Ray screenPointToRay(const glm::vec3& p) const noexcept;
    Ray viewportPointToRay(const glm::vec3& p) const noexcept;

    float getHorizontalFOV() const { return m_fovY * m_aspect; }

    float getVerticalFOV() const { return m_fovY; }

    Frustum getFrustum() const;
    Frustum getFrustum(float nearZ, float farZ) const;
protected:
    bool m_perspective{true};

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
