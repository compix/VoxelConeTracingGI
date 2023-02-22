#include "DebugRenderer.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <engine/util/Timer.h>
#include <engine/geometry/Transform.h>
#include <engine/geometry/BBox.h>
#include <engine/rendering/geometry/MeshBuilder.h>
#include <engine/resource/ResourceManager.h>
#include <engine/rendering/renderer/MeshRenderer.h>
#include <engine/rendering/renderer/MeshRenderers.h>
#include "engine/rendering/voxelConeTracing/Globals.h"
#include <cstddef>

std::shared_ptr<SimpleMeshRenderer> DebugRenderer::m_lineMeshRenderer;
std::shared_ptr<SimpleMeshRenderer> DebugRenderer::m_nonFilledCubeMeshRenderer;
std::shared_ptr<MeshRenderer> DebugRenderer::m_cubeMeshRenderer;
std::shared_ptr<MeshRenderer> DebugRenderer::m_arrowHeadMeshRenderer;
std::shared_ptr<MeshRenderer> DebugRenderer::m_cylinderMeshRenderer;
std::shared_ptr<MeshRenderer> DebugRenderer::m_sphereMeshRenderer;
std::shared_ptr<Shader> DebugRenderer::m_lineShader;
std::shared_ptr<Shader> DebugRenderer::m_debugShader;
std::vector<DebugRenderer::DrawCommand> DebugRenderer::m_drawCommands;
std::vector<DebugRenderer::DrawCommand> DebugRenderer::m_drawQueue;
std::shared_ptr<Shader> DebugRenderer::m_nonFilledCubeShader;
std::shared_ptr<Shader> DebugRenderer::m_instancedQuadShader;
std::unique_ptr<SimpleMeshRenderer> DebugRenderer::m_instancedQuadRenderer;
std::vector<QuadInstanceData> DebugRenderer::m_quadInstanceData;
std::size_t DebugRenderer::m_maxQuadInstances = 100;
std::size_t DebugRenderer::m_quadInstanceIdx = 0;
float DebugRenderer::m_quadInstanceScale = 1.0f;
glm::mat4 DebugRenderer::m_view;
glm::mat4 DebugRenderer::m_proj;
glm::vec3 DebugRenderer::m_cameraPos;
std::vector<DebugRenderInfo> DebugRenderer::m_debugRenderInfoStack;
std::shared_ptr<SimpleMeshRenderer> DebugRenderer::m_triangleRenderer;
std::shared_ptr<Shader> DebugRenderer::m_triangleShader;

struct CamDistComparator
{
    CamDistComparator(const glm::vec3& cameraPos)
        : cameraPos(cameraPos) { }

    bool operator()(const QuadInstanceData& v0, const QuadInstanceData v1) const 
    { 
        return glm::length2(glm::vec3(v0.pos) - cameraPos) > glm::length2(glm::vec3(v1.pos) - cameraPos); 
    }

    glm::vec3 cameraPos;
};

void DebugRenderBatch::reset(float scale)
{
    m_quadInstanceIdx = 0;
    m_quadInstanceScale = scale;
}

void DebugRenderBatch::render(const glm::vec3& pos, const glm::vec4& color, uint8_t faceIdx)
{
    if (m_quadInstanceIdx >= m_maxQuadInstances)
    {
        // Reached max batch capacity -> start a new batch
        renderAll();
        reset(m_quadInstanceScale);
        LOG("Reached max cap.");
    }

    m_quadInstanceData[m_quadInstanceIdx].pos = glm::vec4(pos, float(faceIdx) + 0.5f);
    m_quadInstanceData[m_quadInstanceIdx].color = color;
    ++m_quadInstanceIdx;
}

void DebugRenderBatch::renderAll()
{
    m_instancedQuadShader->bind();
    m_instancedQuadShader->setCamera(m_camera->view(), m_camera->proj());
    m_instancedQuadShader->setFloat("u_scale", m_quadInstanceScale);

    //std::sort(m_quadInstanceData.begin(), m_quadInstanceData.begin() + m_quadInstanceIdx, CamDistComparator(m_cameraPos));

    // Update instance vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_instancedQuadRenderer->getVBO(1));
    //glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * 10, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadInstanceData) * m_quadInstanceIdx, &m_quadInstanceData[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_instancedQuadRenderer->bindAndRenderInstanced(GLsizei(m_quadInstanceIdx));
}

void DebugRenderer::init()
{
    m_debugShader = ResourceManager::getShader("shaders/debug/debug.vert", "shaders/debug/debug.frag", {"in_pos", "in_normal"});
    m_lineShader = ResourceManager::getShader("shaders/simple/lineShader.vert", "shaders/simple/lineShader.frag", {"in_isStart"});
    m_nonFilledCubeShader = ResourceManager::getShader("shaders/simple/pos.vert", "shaders/simple/pos.frag", {"in_pos"});
    m_instancedQuadShader = ResourceManager::getShader("shaders/debug/instancedQuad.vert", "shaders/debug/instancedQuad.frag");
    m_triangleShader = ResourceManager::getShader("shaders/simple/triangle.vert", "shaders/simple/triangle.frag");

    loadInstancedQuad();

    m_cubeMeshRenderer = MeshRenderers::box();
    m_arrowHeadMeshRenderer = MeshRenderers::arrowHead();
    m_cylinderMeshRenderer = MeshRenderers::cylinder();
    m_sphereMeshRenderer = MeshRenderers::sphere();
    m_lineMeshRenderer = MeshRenderers::line();
    m_nonFilledCubeMeshRenderer = MeshRenderers::nonFilledBox();
    m_triangleRenderer = MeshRenderers::triangle();

    GL_ERROR_CHECK();
}

void DebugRenderer::update()
{
    auto it = m_drawCommands.begin();
    while (it != m_drawCommands.end())
    {
        it->drawFunction();
        it->duration -= Time::deltaTime();

        if (it->duration <= 0.f)
            it = m_drawCommands.erase(it);
        else
            ++it;
    }
}

void DebugRenderer::begin(const DebugRenderInfo& debugRenderInfo)
{
    m_debugRenderInfoStack.push_back(debugRenderInfo);
    updateDebugRenderInfo();
}

void DebugRenderer::end()
{
    m_debugRenderInfoStack.pop_back();
    updateDebugRenderInfo();
}

void DebugRenderer::drawLine(glm::vec3 start, glm::vec3 end, const glm::vec3& color, float duration)
{
    m_lineShader->bind();
    m_lineShader->setColor(color);
    m_lineShader->setCamera(m_view, m_proj);

    m_lineShader->setVector("u_startPos", start);
    m_lineShader->setVector("u_endPos", end);

    m_lineMeshRenderer->bindAndRender();

    if (duration > math::EPSILON)
        m_drawCommands.push_back(DrawCommand([=]() { drawLine(start, end, color); }, duration));
}

void DebugRenderer::drawArrow(glm::vec3 start, glm::vec3 end, const glm::vec3& color, float lineThickness, glm::vec3 scale, float duration)
{
    glm::mat3 rotation;
    rotation[2] = glm::normalize(end - start);

    drawVolumetricLine(start, end - rotation[2] * scale.z, color, lineThickness);

    // Draw the arrowhead
    m_debugShader->bind();
    m_debugShader->setColor(glm::vec4(color, 1.0f));
    m_debugShader->setCamera(m_view, m_proj);

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (math::nearEq(abs(rotation[2].y), 1.0f))
        up = glm::vec3(0.0f, 0.0f, 1.0f);

    rotation[0] = glm::normalize(glm::cross(up, rotation[2]));
    rotation[1] = glm::cross(rotation[2], rotation[0]);

    m_debugShader->setModel(glm::translate(end - rotation[2] * scale.z) * glm::mat4(rotation) * glm::scale(scale));

    m_arrowHeadMeshRenderer->render(m_debugShader.get());

    if (duration > math::EPSILON)
        m_drawCommands.push_back(DrawCommand([=]() { drawArrow(start, end, color); }, duration));
}

void DebugRenderer::drawCube(glm::vec3 pos, glm::vec3 scale, glm::vec4 color, float duration)
{
    drawCube(glm::translate(pos) * glm::scale(scale), color, duration);
}

void DebugRenderer::drawCube(glm::mat4 model, glm::vec4 color, float duration, Shader* shader)
{
    shader->bind();
    shader->setColor(color);
    shader->setCamera(m_view, m_proj);
    shader->setModel(model);

    m_cubeMeshRenderer->render(shader);

    if (duration > math::EPSILON)
        m_drawCommands.push_back(DrawCommand([=]() { drawCube(model, color); }, duration));
}

void DebugRenderer::drawNonFilledCube(glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float lineWidth, float duration)
{
    drawNonFilledCube(glm::translate(pos) * glm::scale(scale), color, lineWidth, duration);
}

void DebugRenderer::drawNonFilledCube(glm::mat4 model, glm::vec3 color, float lineWidth, float duration)
{
    m_nonFilledCubeShader->bind();
    m_nonFilledCubeShader->setColor(color);
    m_nonFilledCubeShader->setCamera(m_view, m_proj);
    m_nonFilledCubeShader->setModel(model);

    glLineWidth(lineWidth);
    m_nonFilledCubeMeshRenderer->bindAndRender();
    glLineWidth(1.0f);

    if (duration > math::EPSILON)
        m_drawCommands.push_back(DrawCommand([=]() { drawNonFilledCube(model, color); }, duration));
}

void DebugRenderer::drawCoordinateSystem(const Transform& transform, float duration)
{
    drawArrow(transform.getPosition(), transform.getPosition() + transform.getRight(), glm::vec3(1, 0, 0), 0.005f, glm::vec3(0.05f, 0.05f, 0.1f), duration);
    drawArrow(transform.getPosition(), transform.getPosition() + transform.getUp(), glm::vec3(0, 1, 0), 0.005f, glm::vec3(0.05f, 0.05f, 0.1f), duration);
    drawArrow(transform.getPosition(), transform.getPosition() + transform.getForward(), glm::vec3(0, 0, 1), 0.005f, glm::vec3(0.05f, 0.05f, 0.1f), duration);
}

void DebugRenderer::drawCoordinateSystem(const ComponentPtr<Transform>& transform, float duration)
{
    drawArrow(transform->getPosition(), transform->getPosition() + transform->getRight(), glm::vec3(1, 0, 0), 0.005f, glm::vec3(0.05f, 0.05f, 0.1f), duration);
    drawArrow(transform->getPosition(), transform->getPosition() + transform->getUp(), glm::vec3(0, 1, 0), 0.005f, glm::vec3(0.05f, 0.05f, 0.1f), duration);
    drawArrow(transform->getPosition(), transform->getPosition() + transform->getForward(), glm::vec3(0, 0, 1), 0.005f, glm::vec3(0.05f, 0.05f, 0.1f), duration);
}

void DebugRenderer::drawVolumetricLine(glm::vec3 start, glm::vec3 end, const glm::vec3& color, float thickness, float duration)
{
    m_debugShader->bind();
    m_debugShader->setColor(glm::vec4(color, 1.0f));
    m_debugShader->setCamera(m_view, m_proj);

    glm::mat3 rotation;
    glm::vec3 distance = end - start;
    rotation[2] = glm::normalize(distance);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (math::nearEq(abs(rotation[2].y), 1.0f))
        up = glm::vec3(0.0f, 0.0f, 1.0f);

    rotation[0] = glm::normalize(glm::cross(up, rotation[2]));
    rotation[1] = glm::cross(rotation[2], rotation[0]);

    m_debugShader->setModel(glm::translate(start) * glm::mat4(rotation) * glm::scale(glm::vec3(thickness, thickness, glm::length(distance))));

    m_cylinderMeshRenderer->render(m_debugShader.get());

    if (duration > math::EPSILON)
        m_drawCommands.push_back(DrawCommand([=]() { drawVolumetricLine(start, end, color, thickness); }, duration));
}

void DebugRenderer::drawSphere(const glm::vec3& pos, float radius, const glm::vec4& color, float duration)
{
    bool depthTest = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;

    if (color.a + math::EPSILON < 1.0f)
    {
        if (depthTest)
            glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    m_debugShader->bind();
    m_debugShader->setColor(color);
    m_debugShader->setCamera(m_view, m_proj);
    m_debugShader->setModel(glm::translate(pos) * math::scale(radius * 2.0f));

    m_sphereMeshRenderer->render(m_debugShader.get());

    if (color.a + math::EPSILON < 1.0f)
    {
        glDisable(GL_BLEND);
        if (depthTest)
            glEnable(GL_DEPTH_TEST);
    }

    if (duration > math::EPSILON)
        m_drawCommands.push_back(DrawCommand([=]() { drawSphere(pos, radius, color); }, duration));
}

void DebugRenderer::drawSphere(Shader* shader, const glm::vec3& pos, float radius, const glm::vec4& color, float duration)
{
    shader->bind();
    shader->setColor(color);
    shader->setModel(glm::translate(pos) * math::scale(radius * 2.0f));

    m_sphereMeshRenderer->render(shader);
}

void DebugRenderer::drawCoordinateSystemAxes(const glm::vec3& pos)
{
    drawArrow(pos, pos + glm::vec3(1, 0, 0), glm::vec3(1, 0, 0));
    drawArrow(pos, pos + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    drawArrow(pos, pos + glm::vec3(0, 0, 1), glm::vec3(0, 0, 1));
}

void DebugRenderer::drawTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, bool flipNormal, const glm::vec4& color)
{
    m_triangleShader->bind();
    m_triangleShader->setMatrix("u_modelViewProj", m_proj * m_view);
    m_triangleShader->setVector("u_positions[0]", p0);
    m_triangleShader->setVector("u_positions[1]", p1);
    m_triangleShader->setVector("u_positions[2]", p2);
    m_triangleShader->setVector("u_color", color);
    glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
    if (flipNormal)
        normal *= -1.0f;

    m_triangleShader->setVector("u_normalW", normal);

    m_triangleRenderer->bindAndRender();
}

void DebugRenderer::clearDrawQueue() { m_drawQueue.clear(); }

void DebugRenderer::queueDrawCube(glm::vec3 pos, glm::vec3 scale, glm::vec4 color)
{
    DrawCommand drawCommand = DrawCommand([=]() { drawCube(glm::translate(pos) * glm::scale(scale), color); }, 0.0f);
    drawCommand.pos = pos;
    m_drawQueue.push_back(drawCommand);
}

void DebugRenderer::drawQueuedCommands(bool sortByDistanceToCamera, bool alphaBlending)
{
    if (alphaBlending)
    {
        //glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (sortByDistanceToCamera) { std::sort(m_drawQueue.begin(), m_drawQueue.end(), DrawCommand::DistToCameraSortCondition(m_cameraPos)); }

    for (auto& c : m_drawQueue) { c.drawFunction(); }

    if (alphaBlending)
    {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void DebugRenderer::drawBBox(const BBox& bbox, const glm::vec4& color)
{
    DebugRenderer::drawCube(bbox.center(), bbox.scale(), color);
}

void DebugRenderer::drawCone(const glm::vec3& start, const glm::vec3& end, float apertureRad, const glm::vec4& color)
{
    m_debugShader->bind();
    m_debugShader->setColor(color);
    m_debugShader->setCamera(m_view, m_proj);

    // Set the apex to the origin
    glm::mat4 rotY180 = glm::toMat4(glm::angleAxis(math::PI, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 baseTransform = glm::translate(glm::vec3(0.0f, 0.0f, 1.0f)) * rotY180;

    float scaleZ = glm::length(end - start);
    float scaleX = tanf(apertureRad / 2.0f) * scaleZ;
    float scaleY = scaleX;

    glm::mat3 rotation;
    rotation[2] = glm::normalize(end - start);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (math::nearEq(abs(rotation[2].y), 1.0f))
        up = glm::vec3(0.0f, 0.0f, 1.0f);

    rotation[0] = glm::normalize(glm::cross(up, rotation[2]));
    rotation[1] = glm::cross(rotation[2], rotation[0]);

    m_debugShader->setModel(glm::translate(start) * glm::mat4(rotation) * glm::scale(glm::vec3(scaleX, scaleY, scaleZ)) * baseTransform);

    m_arrowHeadMeshRenderer->render(m_debugShader.get());
}

std::shared_ptr<DebugRenderBatch> DebugRenderer::createBatch()
{
    return std::make_shared<DebugRenderBatch>(MainCamera, m_instancedQuadShader.get(), m_instancedQuadRenderer.get());
}

void DebugRenderer::loadInstancedQuad()
{
    m_quadInstanceData.resize(m_maxQuadInstances);

    for (std::size_t i = 0; i < m_maxQuadInstances; ++i)
    {
        m_quadInstanceData[i].pos.x = i / 10.0f;
        m_quadInstanceData[i].color = glm::vec4(i / 100.0f, 0.0f, 0.0f, 1.0f);
    }

    MeshBuilder builder(4);

    std::vector<glm::vec3> vertices;
    vertices.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
    vertices.push_back(glm::vec3(0.5f, -0.5f, 0.f));
    vertices.push_back(glm::vec3(-0.5f, 0.5f, 0.f));
    vertices.push_back(glm::vec3(0.5f, 0.5f, 0.f));

    VBODescription vboDesc(vertices.size() * sizeof(glm::vec3), &vertices[0]);
    vboDesc.attribute(3, GL_FLOAT);

    VBODescription instanceVBODesc(m_quadInstanceData.size() * sizeof(QuadInstanceData), &m_quadInstanceData[0], GL_STREAM_DRAW);
    instanceVBODesc.attribute(4, GL_FLOAT, 1)
                   .attribute(4, GL_FLOAT, 1);

    builder.createVBO(vboDesc)
           .createVBO(instanceVBODesc)
           .finalize();

    m_instancedQuadRenderer = std::make_unique<SimpleMeshRenderer>(builder, GL_TRIANGLE_STRIP);
}

void DebugRenderer::updateDebugRenderInfo()
{
    if (m_debugRenderInfoStack.size() == 0)
        return;

    auto& info = m_debugRenderInfoStack[m_debugRenderInfoStack.size() - 1];
    setViewProj(info.view, info.proj);
    setCameraPosition(info.cameraPos);
}
