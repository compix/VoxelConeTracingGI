#pragma once
#include <GL/glew.h>
#include <engine/geometry/BBox.h>
#include <engine/rendering/debug/DebugRenderer.h>

struct VoxelRegion;

class Visualizer
{
    struct VertexUint16
    {
        VertexUint16() {}
        VertexUint16(uint16_t x, uint16_t y, uint16_t z)
            :x(x), y(y), z(z) {}

        uint16_t x{ 0 };
        uint16_t y{ 0 };
        uint16_t z{ 0 };
    };

    struct VertexUint16Face
    {
        VertexUint16Face() {}
        VertexUint16Face(uint16_t x, uint16_t y, uint16_t z, uint16_t faceIdx)
            :x(x), y(y), z(z), faceIdx(faceIdx) {}

        uint16_t x{ 0 };
        uint16_t y{ 0 };
        uint16_t z{ 0 };
        uint16_t faceIdx{ 0 };
    };
public:
    Visualizer();

    void visualize3DClipmapGS(GLuint texture, VoxelRegion region, uint32_t clipmapLevel, VoxelRegion prevRegion, bool hasPrevLevel, bool hasMultipleFaces, int numColorComponents);
    void visualize3DClipmapSortedFacesGS(GLuint texture, VoxelRegion region, uint32_t clipmapLevel, VoxelRegion prevRegion, bool hasPrevLevel, bool hasMultipleFaces, int numColorComponents);

    void visualize3DTexture(GLuint texture, const glm::vec3& position, uint32_t clipmapLevel, float padding, float texelSize) const;

    static void showFrustum(const Frustum& frustum);

private:
    std::unique_ptr<SimpleMeshRenderer> initTexture3DRenderer(int resolution);
    std::unique_ptr<SimpleMeshRenderer> initTexture3DFaceRenderer(int resolution);

private:
    std::shared_ptr<Shader> m_voxelVisualizationShader;
    std::shared_ptr<Shader> m_voxelFaceVisualizationShader;
    std::shared_ptr<Shader> m_textureVisualizationShader;
    std::unique_ptr<SimpleMeshRenderer> m_voxelRenderer;
    std::unique_ptr<SimpleMeshRenderer> m_textureRenderer;

    std::vector<VertexUint16Face> m_faceVertices;
    std::unique_ptr<SimpleMeshRenderer> m_voxelFaceRenderer;
};
