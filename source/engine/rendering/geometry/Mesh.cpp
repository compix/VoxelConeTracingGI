#include "Mesh.h"
#include <fstream>
#include <assert.h>
#include <engine/util/convert.h>
#include <engine/util/file.h>
#include "GeometryGenerator.h"
#include <engine/util/util.h>
#include "MeshBuilder.h"

Mesh::~Mesh()
{
    freeGLResources();
}

void Mesh::load(const MeshData& meshData)
{
    ensureCapacity(0);
    SubMesh& subMesh = m_subMeshes[0];
    subMesh.vertices.resize(meshData.vertices.size());
    subMesh.normals.resize(meshData.vertices.size());
    subMesh.tangents.resize(meshData.vertices.size());
    subMesh.bitangents.resize(meshData.vertices.size());
    subMesh.uvs.resize(meshData.vertices.size());
    subMesh.indices = meshData.indices;

    for (size_t i = 0; i < meshData.vertices.size(); ++i)
    {
        subMesh.vertices[i] = meshData.vertices[i].pos;
        subMesh.normals[i] = meshData.vertices[i].normal;
        subMesh.tangents[i] = meshData.vertices[i].tangent;
        subMesh.bitangents[i] = meshData.vertices[i].bitanget;
        subMesh.uvs[i] = meshData.vertices[i].uv;
    }

    finalize();
}

void Mesh::load(const MeshData& meshData, uint32_t vertexAttribFlags)
{
    ensureCapacity(0);
    SubMesh& subMesh = m_subMeshes[0];
    subMesh.vertices.resize(meshData.vertices.size());
    subMesh.normals.resize(meshData.vertices.size());
    subMesh.tangents.resize(meshData.vertices.size());
    subMesh.bitangents.resize(meshData.vertices.size());
    subMesh.uvs.resize(meshData.vertices.size());
    subMesh.indices = meshData.indices;

    for (size_t i = 0; i < meshData.vertices.size(); ++i)
    {
        auto& v = meshData.vertices[i];

        if ((VERTEX_POS & vertexAttribFlags) == VERTEX_POS)
            subMesh.vertices[i] = v.pos;

        if ((VERTEX_NORMAL & vertexAttribFlags) == VERTEX_NORMAL)
            subMesh.normals[i] = v.normal;

        if ((VERTEX_TANGENT & vertexAttribFlags) == VERTEX_TANGENT)
            subMesh.tangents[i] = v.tangent;

        if ((VERTEX_BITANGENT & vertexAttribFlags) == VERTEX_BITANGENT)
            subMesh.bitangents[i] = v.bitanget;

        if ((VERTEX_UV & vertexAttribFlags) == VERTEX_UV)
            subMesh.uvs[i] = v.uv;

        if ((VERTEX_COLOR & vertexAttribFlags) == VERTEX_COLOR)
            subMesh.colors[i] = glm::vec3(v.color);
    }

    finalize();
}

void Mesh::setIndices(Indices indices, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].indices = indices;
}

void Mesh::setVertices(Vertices vertices, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].vertices = vertices;
}

void Mesh::setNormals(Normals normals, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].normals = normals;
}

void Mesh::setTangents(Tangents tangents, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].tangents = tangents;
}

void Mesh::setUVs(UVs uvs, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].uvs = uvs;
}

void Mesh::setColors(Colors colors, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].colors = colors;
}

void Mesh::setRenderMode(GLenum renderMode, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshRenderData[subMeshIdx].renderMode = renderMode;
}

void Mesh::setSubMesh(const SubMesh& subMesh, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx] = subMesh;
}

void Mesh::finalize()
{
    freeGLResources();

    // Go through all submeshes and create ibos/vbos/vaos
    for (size_t mi = 0; mi < m_subMeshes.size(); ++mi)
    {
        auto& subMesh = m_subMeshes[mi];
        auto& renderData = m_subMeshRenderData[mi];
        std::vector<float> vertices;

        if (subMesh.vertices.size() == 0)
            continue;

        // Interleave vertex data
        for (size_t i = 0; i < subMesh.vertices.size(); ++i)
        {
            vertices.push_back(subMesh.vertices[i].x);
            vertices.push_back(subMesh.vertices[i].y);
            vertices.push_back(subMesh.vertices[i].z);

            if (subMesh.normals.size() > 0)
            {
                vertices.push_back(subMesh.normals[i].x);
                vertices.push_back(subMesh.normals[i].y);
                vertices.push_back(subMesh.normals[i].z);
            }

            if (subMesh.tangents.size() > 0)
            {
                vertices.push_back(subMesh.tangents[i].x);
                vertices.push_back(subMesh.tangents[i].y);
                vertices.push_back(subMesh.tangents[i].z);
            }

            if (subMesh.bitangents.size() > 0)
            {
                vertices.push_back(subMesh.bitangents[i].x);
                vertices.push_back(subMesh.bitangents[i].y);
                vertices.push_back(subMesh.bitangents[i].z);
            }

            if (subMesh.uvs.size() > 0)
            {
                vertices.push_back(subMesh.uvs[i].x);
                vertices.push_back(subMesh.uvs[i].y);
            }

            if (subMesh.colors.size() > 0)
            {
                vertices.push_back(subMesh.colors[i].r);
                vertices.push_back(subMesh.colors[i].g);
                vertices.push_back(subMesh.colors[i].b);
            }
        }

        MeshBuilder builder(vertices.size());
        VBODescription vboDesc(vertices.size() * sizeof(float), &vertices[0]);

        // Position
        vboDesc.attribute(3, GL_FLOAT);

        if (subMesh.normals.size() > 0)
            vboDesc.attribute(3, GL_FLOAT);

        if (subMesh.tangents.size() > 0)
            vboDesc.attribute(3, GL_FLOAT);

        if (subMesh.bitangents.size() > 0)
            vboDesc.attribute(3, GL_FLOAT);

        if (subMesh.uvs.size() > 0)
            vboDesc.attribute(2, GL_FLOAT);

        if (subMesh.colors.size() > 0)
            vboDesc.attribute(3, GL_FLOAT);

        builder.createVBO(vboDesc);

        if (subMesh.indices.size() > 0)
            builder.createIBO(subMesh.indices.size(), &subMesh.indices[0]);

        builder.finalize();

        renderData.vbo = builder.getVBO(0);
        renderData.ibo = builder.getIBO();
        renderData.vao = builder.getVAO();
        renderData.renderMode = subMesh.indices.size() > 0 ? GL_TRIANGLES : GL_TRIANGLE_STRIP;
    }
}

void Mesh::setSubMeshes(const std::vector<SubMesh>& subMeshes)
{
    m_subMeshes = subMeshes;
    m_subMeshRenderData.resize(m_subMeshes.size());
}

glm::vec3 Mesh::computeCenter() const
{
    size_t count = 0;
    glm::vec3 center;
    for (auto& subMesh : m_subMeshes)
    {
        for (auto& v : subMesh.vertices)
        {
            center += v;
            ++count;
        }
    }

    return count > 0 ? center / count : center;
}

void Mesh::scale(const glm::vec3& s)
{
    for (auto& subMesh : m_subMeshes)
        for (auto& v : subMesh.vertices)
            v *= s;
}

void Mesh::translate(const glm::vec3& t)
{
    for (auto& subMesh : m_subMeshes)
        for (auto& v : subMesh.vertices)
            v += t;
}

BBox Mesh::computeBBox() const
{
    BBox box;

    for (auto& subMesh : getSubMeshes())
        for (auto& v : subMesh.vertices)
            box.unite(v);

    return box;
}

void Mesh::mapToUnitCube()
{
    BBox meshBBox = util::computeBBox(*this);
    float scaleInv = 1.0f / meshBBox.scale()[meshBBox.maxExtentIdx()];
    glm::vec3 offset = -meshBBox.min();

    for (auto& subMesh : m_subMeshes)
    {
        for (size_t i = 0; i < subMesh.vertices.size(); ++i)
        {
            subMesh.vertices[i] = (subMesh.vertices[i] + offset) * scaleInv;
        }
    }

    finalize();
}

void Mesh::ensureCapacity(SubMeshIndex subMeshIdx)
{
    m_subMeshes.resize(subMeshIdx + 1);
    m_subMeshRenderData.resize(subMeshIdx + 1);
}

void Mesh::ensureIntegrity()
{
#if defined(DEBUG) || defined(_DEBUG)
    for (auto& sm : m_subMeshes)
    {
        assert((sm.normals.size() == 0 || sm.normals.size() == sm.vertices.size()) &&
               (sm.tangents.size() == 0 || sm.tangents.size() == sm.vertices.size()) &&
               (sm.uvs.size() == 0 || sm.uvs.size() == sm.vertices.size()) &&
               (sm.colors.size() == 0 || sm.colors.size() == sm.vertices.size()));
    }
#endif
}

void Mesh::freeGLResources()
{
    for (auto& renderData : m_subMeshRenderData)
    {
        if (renderData.vbo != 0)
            glDeleteBuffers(1, &renderData.vbo);

        if (renderData.ibo != 0)
            glDeleteBuffers(1, &renderData.ibo);

        if (renderData.vao != 0)
            glDeleteVertexArrays(1, &renderData.vao);
    }
}
