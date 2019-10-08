#pragma once
#include <GL/glew.h>
#include <engine/camera/CameraComponent.h>
#include <engine/rendering/geometry/Mesh.h>
#include <engine/rendering/shader/Shader.h>
#include <functional>
#include <memory>
#include <engine/rendering/renderer/MeshRenderer.h>
#include <engine/rendering/renderer/SimpleMeshRenderer.h>

class Transform;
class GLShaderManager;

struct QuadInstanceData
{
    glm::vec4 pos;
    glm::vec4 color;
};

class DebugRenderBatch
{
public:
    DebugRenderBatch(ComponentPtr<CameraComponent> camera, Shader* instancedQuadShader, SimpleMeshRenderer* instancedQuadRenderer)
        : m_camera(camera), m_instancedQuadShader(instancedQuadShader), m_instancedQuadRenderer(instancedQuadRenderer) { m_quadInstanceData.resize(m_maxQuadInstances); }

    void reset(float scale);
    void render(const glm::vec3& pos, const glm::vec4& color, uint8_t faceIdx);
    void renderAll();

private:
    ComponentPtr<CameraComponent> m_camera;
    Shader* m_instancedQuadShader;
    SimpleMeshRenderer* m_instancedQuadRenderer;
    std::vector<QuadInstanceData> m_quadInstanceData;
    size_t m_maxQuadInstances{1000000};
    size_t m_quadInstanceIdx{0};
    float m_quadInstanceScale{1.0f};

};

struct DebugRenderInfo
{
    DebugRenderInfo() { }

    DebugRenderInfo(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos)
        : view(view), proj(proj), cameraPos(cameraPos) { }

    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 cameraPos;
};

/**
* Usage: 
* 1. Set camera (view, proj, pos)
* 2. Render.
* Notes: OpenGL states outside of DebugRenderer can effect the DebugRenderer.
* Rendering is performed "immediately" unless the command is explicitly queued or by providing a duration.
* If a duration is provided make sure to call update() every frame to perform the rendering.
*/
class DebugRenderer
{
    struct DrawCommand
    {
        std::function<void()> drawFunction;
        float duration{0.0f};
        glm::vec3 pos;

        DrawCommand() { }

        DrawCommand(std::function<void()> drawFunction, float duration)
            : drawFunction(drawFunction), duration(duration) { }

        struct DistToCameraSortCondition
        {
            DistToCameraSortCondition(const glm::vec3& cameraPos) : cameraPos(cameraPos) { }

            bool operator()(const DrawCommand& c0, const DrawCommand& c1) const { return glm::length2(cameraPos - c0.pos) > glm::length2(cameraPos - c1.pos); }

            glm::vec3 cameraPos;
        };
    };

public:
    static void init();
    static void update();

    static void begin(const DebugRenderInfo& debugRenderInfo);
    static void end();

    static void drawLine(glm::vec3 start, glm::vec3 end, const glm::vec3& color = glm::vec3(0.5f, 0.5f, 0.5f), float duration = 0.0f);
    static void drawArrow(glm::vec3 start, glm::vec3 end, const glm::vec3& color = glm::vec3(0.5f, 0.5f, 0.5f), 
        float lineThickness = 0.005f, glm::vec3 pointerScale = glm::vec3(0.05f, 0.05f, 0.1f), float duration = 0.0f);
    static void drawCube(glm::vec3 pos, glm::vec3 scale = glm::vec3(1, 1, 1), glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float duration = 0.0f);
    static void drawCube(glm::mat4 model, glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float duration = 0.0f, Shader* shader = m_debugShader.get());

    static void drawNonFilledCube(glm::vec3 pos, glm::vec3 scale = glm::vec3(1, 1, 1), glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f), float lineWidth = 1.0f, float duration = 0.0f);
    static void drawNonFilledCube(glm::mat4 model, glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f), float lineWidth = 1.0f, float duration = 0.0f);

    static void drawCoordinateSystem(const Transform& transform, float duration = 0.f);
    static void drawCoordinateSystem(const ComponentPtr<Transform>& transform, float duration = 0.f);
    static void drawVolumetricLine(glm::vec3 start, glm::vec3 end, const glm::vec3& color = glm::vec3(0.5f, 0.5f, 0.5f), float thickness = 0.005f, float duration = 0.0f);
    static void drawSphere(const glm::vec3& pos, float radius = 0.5f, const glm::vec4& color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float duration = 0.0f);
    static void drawSphere(Shader* shader, const glm::vec3& pos, float radius, const glm::vec4& color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float duration = 0.0f);

    static void drawCoordinateSystemAxes(const glm::vec3& pos = glm::vec3(0.0f));
    static void drawTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, bool flipNormal = false, const glm::vec4& color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    static void clearDrawQueue();
    static void queueDrawCube(glm::vec3 pos, glm::vec3 scale = glm::vec3(1, 1, 1), glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    static void drawQueuedCommands(bool sortByDistanceToCamera, bool alphaBlending);

    static void drawBBox(const BBox& bbox, const glm::vec4& color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    static void drawCone(const glm::vec3& start, const glm::vec3& end, float apertureRad, const glm::vec4& color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    static std::shared_ptr<DebugRenderBatch> createBatch();

    static void loadInstancedQuad();

    static void setMaxQuadInstanced(size_t size) { m_maxQuadInstances = size; }

private:
    static void setViewProj(const glm::mat4& view, const glm::mat4& proj)
    {
        m_view = view;
        m_proj = proj;
    }

    static void setViewMatrix(const glm::mat4& view) { m_view = view; }

    static void setProjectionMatrix(const glm::mat4& proj) { m_proj = proj; }

    static void setCameraPosition(const glm::vec3& pos) { m_cameraPos = pos; }

    static void updateDebugRenderInfo();
private:
    static std::vector<DrawCommand> m_drawCommands;
    static std::vector<DrawCommand> m_drawQueue;

    static std::shared_ptr<SimpleMeshRenderer> m_lineMeshRenderer;
    static std::shared_ptr<SimpleMeshRenderer> m_nonFilledCubeMeshRenderer;
    static std::shared_ptr<SimpleMeshRenderer> m_triangleRenderer;
    static std::shared_ptr<MeshRenderer> m_cubeMeshRenderer;
    static std::shared_ptr<MeshRenderer> m_arrowHeadMeshRenderer;
    static std::shared_ptr<MeshRenderer> m_cylinderMeshRenderer;
    static std::shared_ptr<MeshRenderer> m_sphereMeshRenderer;

    static std::shared_ptr<Shader> m_debugShader;
    static std::shared_ptr<Shader> m_lineShader;
    static std::shared_ptr<Shader> m_nonFilledCubeShader;
    static std::shared_ptr<Shader> m_triangleShader;

    static std::shared_ptr<Shader> m_instancedQuadShader;
    static std::unique_ptr<SimpleMeshRenderer> m_instancedQuadRenderer;
    static std::vector<QuadInstanceData> m_quadInstanceData;
    static size_t m_maxQuadInstances;
    static size_t m_quadInstanceIdx;
    static float m_quadInstanceScale;

    static std::vector<DebugRenderInfo> m_debugRenderInfoStack;

    static glm::mat4 m_view;
    static glm::mat4 m_proj;
    static glm::vec3 m_cameraPos;
};
