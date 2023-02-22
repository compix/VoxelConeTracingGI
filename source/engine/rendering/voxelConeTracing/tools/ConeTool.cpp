#include "ConeTool.h"
#include <string>
#include <engine/util/util.h>
#include <glm/ext.hpp>
#include <engine/util/file.h>
#include <engine/rendering/debug/DebugRenderer.h>
#include <fstream>
#include <engine/rendering/Screen.h>
#include <engine/input/Input.h>
#include "engine/rendering/voxelConeTracing/Globals.h"
#include <cstddef>

std::vector<glm::vec3> getSpherePoints(const std::string& spherePoints, const std::string& filePath)
{
    std::vector<glm::vec3> points;

    auto spherePointsVec = util::splitWhitespace(spherePoints);

    try
    {
        for (std::size_t i = 0; i < spherePointsVec.size(); i += 3)
        {
            float x = std::stof(spherePointsVec[i]);
            float y = std::stof(spherePointsVec[i + 1]);
            float z = std::stof(spherePointsVec[i + 2]);

            points.push_back(glm::vec3(x, y, z));
        }
    }
    catch (const std::exception&)
    {
        LOG_ERROR("Could not parse " << filePath);
        points.clear();
    }

    return points;
}

ConeTool::ConeTool()
{
    m_camera.setPerspective(45.0f, m_imageSize.x, m_imageSize.y, 0.3f, 30.0f);
    m_camera.setPosition(glm::vec3(0.0f, 0.0f, -3.0f));
    m_camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    GLsizei imageSizeX = static_cast<GLsizei>(m_imageSize.x);
    GLsizei imageSizeY = static_cast<GLsizei>(m_imageSize.y);

    m_framebuffer = std::make_unique<Framebuffer>(imageSizeX, imageSizeY, true, GL_UNSIGNED_BYTE, true);
    m_msaaFramebuffer = std::make_unique<Framebuffer>();

    m_msaaFramebuffer->bind();
    auto msaaRenderTexture = std::make_shared<Texture2D>();
    msaaRenderTexture->createMultisampled(8, imageSizeX, imageSizeY, GL_RGBA8);
    m_msaaFramebuffer->attachRenderTexture2DMultisample(msaaRenderTexture);
    m_msaaFramebuffer->setDrawBuffers();
    auto msaaDepthTexture = std::make_shared<Texture2D>();
    msaaDepthTexture->createMultisampled(8, imageSizeX, imageSizeY, GL_DEPTH_COMPONENT24);
    m_msaaFramebuffer->attachDepthBufferMultisample(msaaDepthTexture);
    m_msaaFramebuffer->checkFramebufferStatus();
    m_msaaFramebuffer->unbind();

    m_window.open = false;
    m_window.minSize = ImVec2(250, 250);

    std::string defaultPath = ASSET_ROOT_FOLDER + std::string("spherePoints/cones16.txt");
    loadSpherePoints(defaultPath);
}

void ConeTool::loadSpherePoints(const std::string& path)
{
    if (!file::exists(path))
        return;

    m_loadedSpherePoints = getSpherePoints(file::readAsString(path), path);
}

void ConeTool::renderCones()
{
    GLsizei imageSizeX = static_cast<GLsizei>(m_imageSize.x);
    GLsizei imageSizeY = static_cast<GLsizei>(m_imageSize.y);

    if (!math::nearEq(m_imageSize, m_lastImageSize))
    {
        m_imageSize = glm::max(m_imageSize, glm::vec2(1.0f, 1.0f));
        m_camera.setPerspective(45.0f, m_imageSize.x, m_imageSize.y, 0.3f, 30.0f);
        m_framebuffer->resize(imageSizeX, imageSizeY);
        m_msaaFramebuffer->resize(imageSizeX, imageSizeY);
        m_lastImageSize = m_imageSize;
    }

    m_camera.setViewport(m_imageMin.x, m_imageMin.y, m_imageSize.x, m_imageSize.y);

    m_camera.updateViewMatrix();
    auto viewport = m_camera.getViewport();

    // Render the cones to a framebuffer
    m_msaaFramebuffer->begin(static_cast<GLsizei>(viewport.width()), static_cast<GLsizei>(viewport.height()));
    glEnable(GL_MULTISAMPLE);
    glClearColor(1.0f, 1.0, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DebugRenderInfo debugRenderInfo(m_camera.view(), m_camera.proj(), m_camera.getPosition());
    DebugRenderer::begin(debugRenderInfo);

    auto rotationMatrix = glm::toMat3(m_rotator.getRotation());
    glm::vec3 right = rotationMatrix[0];
    glm::vec3 up = rotationMatrix[1];
    glm::vec3 normal = rotationMatrix[2];

    DebugRenderer::drawArrow(glm::vec3(0.0f), right, glm::vec3(1.0f, 0.0f, 0.0f));
    DebugRenderer::drawArrow(glm::vec3(0.0f), up, glm::vec3(0.0f, 1.0f, 0.0f));
    DebugRenderer::drawArrow(glm::vec3(0.0f), normal, glm::vec3(0.0f, 0.0f, 1.0f));

    float angleRad = math::toRadians(m_coneAngleDeg);

    for (auto& direction : m_coneDirections)
    {
        glm::vec3 coneDirection = glm::normalize(direction.x * right + direction.y * up + normal * direction.z);
        DebugRenderer::drawCone(glm::vec3(0.f), coneDirection, angleRad, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    }

    DebugRenderer::end();
    m_msaaFramebuffer->end();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *m_framebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_msaaFramebuffer);
    glBlitFramebuffer(0, 0, imageSizeX, imageSizeY, 0, 0, imageSizeX, imageSizeY, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void ConeTool::generateHemisphereCones()
{
    m_coneDirections.clear();

    glm::vec3 axis = MAIN_AXES[m_axisIdx];

    for (std::size_t i = 0; i < m_loadedSpherePoints.size(); ++i)
    {
        glm::vec3 direction = glm::normalize(m_loadedSpherePoints[i]);

        if (m_hemisphere)
        {
            float dp = glm::dot(direction, axis);
            if (dp > 0.0f)
            {
                m_coneDirections.push_back(direction);
            }
        }
        else
            m_coneDirections.push_back(direction);
    }
}

void ConeTool::generateGLSLCodeForDirections(const glm::vec3& normal, const std::vector<glm::vec3>& directions) const
{
    std::string path = ASSET_ROOT_FOLDER + std::string("directionsGLSL.txt");
    std::ofstream stream(path, std::ios::out);

    if (!stream.is_open())
    {
        LOG_ERROR("Could not open file: " << path);
        return;
    }

    // Save the floats:
    stream << "// Sphere points as floats: \n";
    for (std::size_t i = 0; i < directions.size(); ++i)
    {
        glm::vec3 d = directions[i];
        stream << d.x << "\n";
        stream << d.y << "\n";
        stream << d.z << "\n";
    }

    stream << "\n";
    stream << "// Assuming the " << (m_axisIdx == 0 ? "X" : (m_axisIdx == 1 ? "Y" : "Z")) << " axis as the normal. \n";
    stream << "const int DIFFUSE_CONE_COUNT = " << directions.size() << ";\n";
    stream << "const float DIFFUSE_CONE_APERTURE = " << math::toRadians(m_coneAngleDeg) << ";\n\n";
    stream << "const vec3 DIFFUSE_CONE_DIRECTIONS[" << directions.size() << "] = {\n";

    for (std::size_t i = 0; i < directions.size(); ++i)
    {
        glm::vec3 d = directions[i];
        stream << "    vec3(" << d.x << ", " << d.y << ", " << d.z << ")" << (i + 1 < directions.size() ? ",\n" : "\n");
    }

    stream << "};\n\n";

    stream.close();
}

void ConeTool::generateGLSLCodeForDirections(const glm::vec3& normal) const
{
    generateGLSLCodeForDirections(normal, m_coneDirections);
}

void ConeTool::update()
{
    generateHemisphereCones();
    renderCones();

    // GUI
    m_window.begin();
    auto viewport = m_camera.getViewport();

    // Axis selection
    static const char* axes[] = {
            "X", "Y", "Z"
        };

    ImGui::PushItemWidth(50.0f);
    ImGui::Combo("Axis", &m_axisIdx, axes, 3);
    ImGui::SameLine();
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("Cone Angle", &m_coneAngleDeg, 0.0f, 90.0f);
    ImGui::SameLine();
    ImGui::PopItemWidth();
    ImGui::Checkbox("Hemisphere", &m_hemisphere);
    ImGui::SameLine();
    if (ImGui::Button("Generate GLSL Code", ImVec2(150, 20)))
        generateGLSLCodeForDirections(MAIN_AXES[m_axisIdx]);

    ImGui::Text("Sphere Points Path:");
    ImGui::SameLine();
    ImGui::PushItemWidth(312.0f);
    ImGui::InputText("", m_path, 255);
    ImGui::SameLine();
    ImGui::PopItemWidth();
    if (ImGui::Button("Load", ImVec2(60, 20)))
        loadSpherePoints(m_path);

    m_imageSize.x = m_window.size.x;
    m_imageSize.y = m_window.size.y - 100.0f;

    ImGui::Image(ImTextureID(uintptr_t(m_framebuffer->getRenderTexture())), ImVec2(m_imageSize.x, m_imageSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
    auto itemRectMin = ImGui::GetItemRectMin();
    m_imageMin = glm::vec2(itemRectMin.x, Screen::getHeight() - (itemRectMin.y + m_imageSize.y));

    if (Input::leftDrag().isDragging() && (ImGui::IsItemHovered() || m_startedDragging))
    {
        m_startedDragging = true;
        m_rotator.onRotationUpdate(viewport, m_camera.viewInverse(), Input::leftDrag().getStartDragPos(), Input::leftDrag().getCurDragPos());
        m_window.flags |= ImGuiWindowFlags_NoMove;
    }
    else
    {
        m_startedDragging = false;
        m_rotator.onIdle();
        m_window.flags &= ~ImGuiWindowFlags_NoMove;
    }

    m_window.end();
}
