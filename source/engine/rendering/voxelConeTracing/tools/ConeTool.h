#pragma once
#include <vector>
#include <engine/util/ArcballRotator.h>
#include <engine/camera/Camera.h>
#include <engine/rendering/Framebuffer.h>
#include "engine/gui/GUIElements.h"

class ConeTool
{
public:
    ConeTool();

    void generateHemisphereCones();
    void generateGLSLCodeForDirections(const glm::vec3& normal, const std::vector<glm::vec3>& directions) const;
    void generateGLSLCodeForDirections(const glm::vec3& normal) const;

    void setConeAngle(float angle) { m_coneAngleDeg = angle; }
    void setAxisIdx(int axisIdx) { m_axisIdx = axisIdx; }

    void update();

    bool isOpen() const { return m_window.open; }
    void setOpen(bool open) { m_window.open = open; }

    void loadSpherePoints(const std::string& path);

    const std::vector<glm::vec3>& getConeDirections() const { return m_coneDirections; }
private:
    void renderCones();

private:
    std::vector<glm::vec3> m_loadedSpherePoints;
    std::vector<glm::vec3> m_coneDirections;
    float m_coneAngleDeg{45.0f};
    int m_axisIdx{1};

    ArcballRotator m_rotator;
    Camera m_camera;
    GUIWindow m_window{"Cone Tool"};
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Framebuffer> m_msaaFramebuffer;
    glm::vec2 m_imageSize{500.0f};
    glm::vec2 m_lastImageSize{500.0f};
    glm::vec2 m_imageMin;
    glm::vec2 m_lastImageMin;

    bool m_startedDragging{false};
    bool m_hemisphere{true};
    char m_path[255]{ "" };
};
