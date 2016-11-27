#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <engine/util/Logger.h>
#include "GeometryGenerator.h"
#include "engine/geometry/BBox.h"

struct MeshData;

#define VERTEX_POS 0
#define VERTEX_NORMAL 2
#define VERTEX_TANGENT 4
#define VERTEX_BITANGENT 8
#define VERTEX_UV 16
#define VERTEX_COLOR 32

using IndexType = GLuint;

using SubMeshIndex = uint16_t;
using Indices = std::vector<IndexType>;
using Vertices = std::vector<glm::vec3>;
using Normals = std::vector<glm::vec3>;
using Tangents = std::vector<glm::vec3>;
using Bitangents = std::vector<glm::vec3>;
using UVs = std::vector<glm::vec2>;
using Colors = std::vector<glm::vec3>;

class Mesh
{
public:
    struct SubMesh
    {
        Indices indices;
        Vertices vertices;
        Normals normals;
        Tangents tangents;
        Bitangents bitangents;
        UVs uvs;
        Colors colors;
    };

    struct SubMeshRenderData
    {
        GLuint vbo{0};
        GLuint ibo{0};
        GLuint vao{0};
        GLenum renderMode{GL_TRIANGLES};
    };

    friend class MeshRenderer;
    friend class MeshRenderSystem;

    Mesh() { }

    ~Mesh();

    void load(const MeshData& meshData);
    void load(const MeshData& meshData, uint32_t vertexAttribFlags);

    void setIndices(Indices indices, SubMeshIndex subMeshIdx);
    void setVertices(Vertices vertices, SubMeshIndex subMeshIdx);
    void setNormals(Normals normals, SubMeshIndex subMeshIdx);
    void setTangents(Tangents tangents, SubMeshIndex subMeshIdx);
    void setUVs(UVs uvs, SubMeshIndex subMeshIdx);
    void setColors(Colors colors, SubMeshIndex subMeshIdx);
    void setRenderMode(GLenum renderMode, SubMeshIndex subMeshIdx);
    void setSubMesh(const SubMesh& subMesh, SubMeshIndex subMeshIdx);
    void finalize();

    void setSubMeshes(const std::vector<SubMesh>& subMeshes);

    const std::vector<SubMesh>& getSubMeshes() const { return m_subMeshes; }

    glm::vec3 computeCenter() const;

    void scale(const glm::vec3& s);
    void translate(const glm::vec3& t);

    BBox computeBBox() const;

    /**
    * @fn  void Builder::mapToUnitCube();
    *
    * @brief   Maps the mesh vertices to a unit cube.
    */
    void mapToUnitCube();

private:
    void ensureCapacity(SubMeshIndex subMeshIdx);
    void ensureIntegrity();
    void freeGLResources();

private:
    std::vector<SubMesh> m_subMeshes;
    std::vector<SubMeshRenderData> m_subMeshRenderData;
};
