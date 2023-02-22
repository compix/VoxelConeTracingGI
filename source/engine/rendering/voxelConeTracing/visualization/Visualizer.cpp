#include "Visualizer.h"
#include <engine/rendering/debug/DebugRenderer.h>
#include <engine/resource/ResourceManager.h>
#include <engine/rendering/geometry/MeshBuilder.h>
#include "engine/rendering/voxelConeTracing/Globals.h"
#include "engine/rendering/voxelConeTracing/VoxelRegion.h"
#include "engine/rendering/voxelConeTracing/settings/VoxelConeTracingSettings.h"
#include <cstddef>

Visualizer::Visualizer()
{
    m_voxelVisualizationShader = ResourceManager::getShader("shaders/voxelConeTracing/visualization/voxelVisualization.vert",
                                                            "shaders/voxelConeTracing/visualization/voxelVisualization.frag",
                                                            "shaders/voxelConeTracing/visualization/voxelVisualization.geom");

    m_textureVisualizationShader = ResourceManager::getShader("shaders/voxelConeTracing/visualization/texture3DVisualization.vert",
                                                              "shaders/voxelConeTracing/visualization/texture3DVisualization.frag",
                                                              "shaders/voxelConeTracing/visualization/texture3DVisualization.geom");

    //m_voxelFaceVisualizationShader = ResourceManager::getShader("shaders/voxelConeTracing/visualization/voxelFaceVisualization.vert",
    //                                                            "shaders/voxelConeTracing/visualization/voxelFaceVisualization.frag",
    //                                                            "shaders/voxelConeTracing/visualization/voxelFaceVisualization.geom");

    m_voxelRenderer = initTexture3DRenderer(VOXEL_RESOLUTION);
    //m_voxelFaceRenderer = initTexture3DFaceRenderer(VOXEL_RESOLUTION);
    m_textureRenderer = initTexture3DRenderer(VOXEL_RESOLUTION + 2);
}

void Visualizer::visualize3DClipmapGS(GLuint texture, VoxelRegion region, uint32_t clipmapLevel, VoxelRegion prevRegion, 
    bool hasPrevLevel, bool hasMultipleFaces, int numColorComponents)
{
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_voxelVisualizationShader->bind();
    m_voxelVisualizationShader->bindTexture3D(texture, "u_3dTexture");
    m_voxelVisualizationShader->setVector("u_volumeMin", region.getMinPosWorld());
    m_voxelVisualizationShader->setInt("u_clipmapResolution", int(VOXEL_RESOLUTION));
    m_voxelVisualizationShader->setInt("u_clipmapLevel", int(clipmapLevel));
    m_voxelVisualizationShader->setFloat("u_voxelSize", region.voxelSize);
    m_voxelVisualizationShader->setMatrix("u_viewProj", MainCamera->viewProj());

    m_voxelVisualizationShader->setVectori("u_imageMin", region.getMinPosImage(region.extent));
    m_voxelVisualizationShader->setVectori("u_regionMin", region.minPos);
    // Transform prev region into the voxel coordinate system of the current region
    m_voxelVisualizationShader->setVectori("u_prevRegionMin", prevRegion.minPos / 2);
    m_voxelVisualizationShader->setVectori("u_prevRegionMax", prevRegion.getMaxPos() / 2);

    m_voxelVisualizationShader->setFloat("u_alpha", VISUALIZATION_SETTINGS.voxelVisualizationAlpha);
    m_voxelVisualizationShader->setFloat("u_borderWidth", VISUALIZATION_SETTINGS.borderWidth);
    m_voxelVisualizationShader->setVector("u_borderColor", VISUALIZATION_SETTINGS.borderColor);

    m_voxelVisualizationShader->setInt("u_hasPrevClipmapLevel", hasPrevLevel ? 1 : 0);
    m_voxelVisualizationShader->setInt("u_hasMultipleFaces", hasMultipleFaces ? 1 : 0);
    m_voxelVisualizationShader->setInt("u_numColorComponents", numColorComponents);
    m_voxelVisualizationShader->setVector("u_eyePosW", MainCamera->getPosition());

    m_voxelRenderer->bindAndRender();

    glDisable(GL_BLEND);
}

void Visualizer::visualize3DClipmapSortedFacesGS(GLuint texture, VoxelRegion region, uint32_t clipmapLevel, 
                                                 VoxelRegion prevRegion, bool hasPrevLevel, bool hasMultipleFaces, int numColorComponents)
{
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    glm::vec3 cameraPos = MainCamera->getPosition();
    float voxelSize = region.voxelSize;
    glm::ivec3 regionMin = region.minPos;
    glm::vec3 faceOffsets[6]{glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.5f, 0.0f, 0.0f), 
                             glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f),
                             glm::vec3(0.0f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 0.5f) };

    std::sort(m_faceVertices.begin(), m_faceVertices.end(), [cameraPos, voxelSize, regionMin, &faceOffsets](const VertexUint16Face& v0, const VertexUint16Face& v1)
    {
        return glm::length2(cameraPos - ((glm::vec3(regionMin) + glm::vec3(v0.x, v0.y, v0.z) + faceOffsets[v0.faceIdx]) * voxelSize)) >
               glm::length2(cameraPos - ((glm::vec3(regionMin) + glm::vec3(v1.x, v1.y, v1.z) + faceOffsets[v1.faceIdx]) * voxelSize));
    });

    glBindBuffer(GL_ARRAY_BUFFER, m_voxelFaceRenderer->getVBO(0));
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexUint16Face) * m_faceVertices.size(), &m_faceVertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_voxelFaceVisualizationShader->bind();
    m_voxelFaceVisualizationShader->bindTexture3D(texture, "u_3dTexture");
    m_voxelFaceVisualizationShader->setVector("u_volumeMin", region.getMinPosWorld());
    m_voxelFaceVisualizationShader->setInt("u_clipmapResolution", int(VOXEL_RESOLUTION));
    m_voxelFaceVisualizationShader->setInt("u_clipmapLevel", int(clipmapLevel));
    m_voxelFaceVisualizationShader->setFloat("u_voxelSize", region.voxelSize);
    m_voxelFaceVisualizationShader->setMatrix("u_viewProj", MainCamera->viewProj());

    m_voxelFaceVisualizationShader->setVectori("u_imageMin", region.getMinPosImage(region.extent));
    m_voxelFaceVisualizationShader->setVectori("u_regionMin", region.minPos);
    // Transform prev region into the voxel coordinate system of the current region
    m_voxelFaceVisualizationShader->setVectori("u_prevRegionMin", prevRegion.minPos / 2);
    m_voxelFaceVisualizationShader->setVectori("u_prevRegionMax", prevRegion.getMaxPos() / 2);

    m_voxelFaceVisualizationShader->setFloat("u_alpha", VISUALIZATION_SETTINGS.voxelVisualizationAlpha);
    m_voxelFaceVisualizationShader->setFloat("u_borderWidth", VISUALIZATION_SETTINGS.borderWidth);
    m_voxelFaceVisualizationShader->setVector("u_borderColor", VISUALIZATION_SETTINGS.borderColor);

    m_voxelFaceVisualizationShader->setInt("u_hasPrevClipmapLevel", hasPrevLevel ? 1 : 0);
    m_voxelFaceVisualizationShader->setInt("u_hasMultipleFaces", hasMultipleFaces ? 1 : 0);
    m_voxelFaceVisualizationShader->setInt("u_numColorComponents", numColorComponents);
    m_voxelFaceVisualizationShader->setVector("u_eyePosW", MainCamera->getPosition());

    m_voxelFaceRenderer->bindAndRenderInstanced(static_cast<GLsizei>(m_faceVertices.size()));

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void Visualizer::visualize3DTexture(GLuint texture, const glm::vec3& position, uint32_t visualizeClipNum, float padding, float texelSize) const
{
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_textureVisualizationShader->bind();
    m_textureVisualizationShader->bindTexture3D(texture, "u_3dTexture");
    m_textureVisualizationShader->setMatrix("u_viewProj", MainCamera->viewProj());

    m_textureVisualizationShader->setInt("u_clipmapLevel", int(visualizeClipNum));
    m_textureVisualizationShader->setInt("u_faceCount", 6);
    m_textureVisualizationShader->setVectori("u_resolution", glm::ivec3(VOXEL_RESOLUTION + 2));
    m_textureVisualizationShader->setFloat("u_texelSize", texelSize);
    m_textureVisualizationShader->setFloat("u_padding", padding);
    m_textureVisualizationShader->setVector("u_position", position);

    m_textureVisualizationShader->setFloat("u_alpha", VISUALIZATION_SETTINGS.voxelVisualizationAlpha);
    m_textureVisualizationShader->setFloat("u_borderWidth", VISUALIZATION_SETTINGS.borderWidth);
    m_textureVisualizationShader->setVector("u_borderColor", VISUALIZATION_SETTINGS.borderColor);

    m_textureRenderer->bindAndRender();

    glDisable(GL_BLEND);
}

void Visualizer::showFrustum(const Frustum& frustum)
{
    DebugRenderer::drawVolumetricLine(frustum.data.nearBottomLeft, frustum.data.nearBottomRight);
    DebugRenderer::drawVolumetricLine(frustum.data.nearBottomRight, frustum.data.nearTopRight);
    DebugRenderer::drawVolumetricLine(frustum.data.nearTopRight, frustum.data.nearTopLeft);
    DebugRenderer::drawVolumetricLine(frustum.data.nearTopLeft, frustum.data.nearBottomLeft);

    DebugRenderer::drawVolumetricLine(frustum.data.farBottomLeft, frustum.data.farBottomRight);
    DebugRenderer::drawVolumetricLine(frustum.data.farBottomRight, frustum.data.farTopRight);
    DebugRenderer::drawVolumetricLine(frustum.data.farTopRight, frustum.data.farTopLeft);
    DebugRenderer::drawVolumetricLine(frustum.data.farTopLeft, frustum.data.farBottomLeft);

    DebugRenderer::drawVolumetricLine(frustum.data.nearBottomLeft, frustum.data.farBottomLeft);
    DebugRenderer::drawVolumetricLine(frustum.data.nearBottomRight, frustum.data.farBottomRight);
    DebugRenderer::drawVolumetricLine(frustum.data.nearTopRight, frustum.data.farTopRight);
    DebugRenderer::drawVolumetricLine(frustum.data.nearTopLeft, frustum.data.farTopLeft);
}

std::unique_ptr<SimpleMeshRenderer> Visualizer::initTexture3DRenderer(int resolution)
{
    // Create the mesh for the voxel visualization
    std::size_t vertexCount = std::size_t(resolution * resolution * resolution);
    MeshBuilder meshBuilder(vertexCount);

    std::vector<VertexUint16> vertices;
    vertices.reserve(vertexCount);

    for (uint16_t z = 0; z < resolution; ++z)
    {
        for (uint16_t y = 0; y < resolution; ++y)
        {
            for (uint16_t x = 0; x < resolution; ++x)
            {
                vertices.push_back(VertexUint16(x, y, z));
            }
        }
    }

    VBODescription vboDesc(sizeof(VertexUint16) * vertexCount, &vertices[0]);
    vboDesc.attribute(3, GL_UNSIGNED_SHORT);
    meshBuilder.createVBO(vboDesc);
    meshBuilder.finalize();

    return std::move(std::make_unique<SimpleMeshRenderer>(meshBuilder, GL_POINTS));
}

std::unique_ptr<SimpleMeshRenderer> Visualizer::initTexture3DFaceRenderer(int resolution)
{
    // m_voxelFaceRenderer
    // Create the mesh for the voxel visualization
    std::size_t vertexCount = std::size_t(resolution * resolution * resolution) * 6;
    MeshBuilder meshBuilder(1);

    m_faceVertices.reserve(vertexCount);

    for (uint16_t i = 0; i < 6; ++i)
    {
        for (uint16_t z = 0; z < resolution; ++z)
        {
            for (uint16_t y = 0; y < resolution; ++y)
            {
                for (uint16_t x = 0; x < resolution; ++x)
                {
                    m_faceVertices.push_back(VertexUint16Face(x, y, z, i));
                }
            }
        }
    }

    VBODescription vboDesc(sizeof(VertexUint16Face) * vertexCount, &m_faceVertices[0]);
    vboDesc.attribute(4, GL_UNSIGNED_SHORT, 1);
    meshBuilder.createVBO(vboDesc);
    meshBuilder.finalize();

    return std::move(std::make_unique<SimpleMeshRenderer>(meshBuilder, GL_POINTS));
}
