#include "GeometryGenerator.h"
#include <GL/gl.h>
#include <engine/util/Logger.h>
#include <engine/rendering/debug/DebugRenderer.h>
#include <cstddef>

void MeshData::computeBitangets()
{
    // Compute bitangets
    for (std::size_t i = 0; i < vertices.size(); ++i)
        vertices[i].bitanget = glm::cross(vertices[i].normal, vertices[i].tangent);
}

/**
* Algorithm adapted from: Introduction To 3D Game Programming With DirectX 11 by Frank D. Luna
*/
MeshData GeometryGenerator::createBox(float width, float height, float depth)
{
    MeshData meshData;

    Vertex vertices[24];

    float w2 = 0.5f * width;
    float h2 = 0.5f * height;
    float d2 = 0.5f * depth;

    // Fill in the front face vertex data.
    vertices[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the back face vertex data.
    vertices[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertices[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Fill in the top face vertex data.
    vertices[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the bottom face vertex data.
    vertices[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertices[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Fill in the left face vertex data.
    vertices[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    vertices[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    vertices[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    vertices[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // Fill in the right face vertex data.
    vertices[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    vertices[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    vertices[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    vertices[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 24; ++i)
        meshData.vertices.push_back(vertices[i]);

    meshData.computeBitangets();

    GLuint indices[36];

    // Back Face
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;

    // Front Face
    indices[6] = 4;
    indices[7] = 5;
    indices[8] = 6;
    indices[9] = 4;
    indices[10] = 6;
    indices[11] = 7;

    // Bottom Face
    indices[12] = 8;
    indices[13] = 9;
    indices[14] = 10;
    indices[15] = 8;
    indices[16] = 10;
    indices[17] = 11;

    // Top Face
    indices[18] = 12;
    indices[19] = 13;
    indices[20] = 14;
    indices[21] = 12;
    indices[22] = 14;
    indices[23] = 15;

    // Right Face
    indices[24] = 16;
    indices[25] = 17;
    indices[26] = 18;
    indices[27] = 16;
    indices[28] = 18;
    indices[29] = 19;

    // Left Face
    indices[30] = 20;
    indices[31] = 21;
    indices[32] = 22;
    indices[33] = 20;
    indices[34] = 22;
    indices[35] = 23;

    for (int i = 0; i < 36; ++i)
        meshData.indices.push_back(indices[i]);

    meshData.indexed = true;
    meshData.renderMode = GL_TRIANGLES;

    return meshData;
}

/**
 * Algorithm adapted from: Introduction To 3D Game Programming With DirectX 11 by Frank D. Luna
 */
MeshData GeometryGenerator::createSphere(float radius, GLuint sliceCount, GLuint stackCount)
{
    MeshData meshData;

    //
    // Compute the vertices starting at the top pole and moving down the stacks.
    //

    // Poles: note that there will be texture coordinate distortion as there is
    // not a unique point on the texture map to assign to the pole when mapping
    // a rectangular texture onto a sphere.
    Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    meshData.vertices.push_back(topVertex);

    float phiStep = math::PI / stackCount;
    float thetaStep = math::PI2 / sliceCount;

    // Compute vertices for each stack ring (do not count the poles as rings).
    for (GLuint i = 1; i <= stackCount - 1; ++i)
    {
        float phi = i * phiStep;

        // Vertices of ring.
        for (GLuint j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            Vertex v;

            // spherical to cartesian
            v.pos.x = radius * sinf(phi) * cosf(theta);
            v.pos.y = radius * cosf(phi);
            v.pos.z = radius * sinf(phi) * sinf(theta);

            // Partial derivative of P with respect to theta
            v.tangent = glm::normalize(glm::vec3(-radius * sinf(phi) * sinf(theta), 0.0f, radius * sinf(phi) * cosf(theta)));
            v.normal = glm::normalize(v.pos);

            v.uv.x = theta / math::PI2;
            v.uv.y = phi / math::PI;

            meshData.vertices.push_back(v);
        }
    }

    meshData.vertices.push_back(bottomVertex);

    meshData.computeBitangets();

    //
    // Compute indices for top stack.  The top stack was written first to the vertex buffer
    // and connects the top pole to the first ring.
    //

    for (GLuint i = 1; i <= sliceCount; ++i)
    {
        meshData.indices.push_back(0);
        meshData.indices.push_back(i + 1);
        meshData.indices.push_back(i);
    }

    //
    // Compute indices for inner stacks (not connected to poles).
    //

    // Offset the indices to the index of the first vertex in the first ring.
    // This is just skipping the top pole vertex.
    GLuint baseIndex = 1;
    GLuint ringVertexCount = sliceCount + 1;
    for (GLuint i = 0; i < stackCount - 2; ++i)
    {
        for (GLuint j = 0; j < sliceCount; ++j)
        {
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j);
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    //
    // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
    // and connects the bottom pole to the bottom ring.
    //

    // South pole vertex was added last.
    GLuint southPoleIndex = GLuint(meshData.vertices.size() - 1);

    // Offset the indices to the index of the first vertex in the last ring.
    baseIndex = southPoleIndex - ringVertexCount;

    for (GLuint i = 0; i < sliceCount; ++i)
    {
        meshData.indices.push_back(southPoleIndex);
        meshData.indices.push_back(baseIndex + i);
        meshData.indices.push_back(baseIndex + i + 1);
    }

    meshData.renderMode = GL_TRIANGLES;
    meshData.indexed = true;

    return meshData;
}

MeshData GeometryGenerator::createArrowHead()
{
    /// Simple arrowhead that points towards the z axis in its local coordinate system

    MeshData meshData;

    const GLuint circleVertexNum = 25;
    const GLuint vertexNum = circleVertexNum * 3 + 1;
    meshData.vertices.resize(vertexNum);
    const GLuint trianglesNum = (circleVertexNum - 1) * 2;
    meshData.indices.resize(trianglesNum * 3);

    // bot circle center
    meshData.vertices[vertexNum - 1] = Vertex(0, 0, 0, 0, 0, -1, 0, 1, 0, 0.5f, 0.5f);

    float angleStep = math::PI2 / (circleVertexNum - 1);

    GLuint circleStart = circleVertexNum;
    GLuint circleStart2 = circleStart + circleVertexNum;

    // Pointer
    glm::vec3 arrowTip(0.f, 0.f, 1.f);

    float tl = sinf(math::PI_DIV_4);

    for (std::size_t i = 0; i < circleVertexNum; ++i)
    {
        float angle = angleStep * i;
        float c = cosf(angle);
        float s = sinf(angle);

        glm::vec3 p(c, s, 0.0f);

        glm::vec3 tangent = glm::normalize(arrowTip - p);
        glm::vec3 normal = glm::normalize(p + tl * tangent);

        meshData.vertices[i] = Vertex(arrowTip, normal, tangent, glm::vec2(c, s));
        meshData.vertices[circleStart + i] = Vertex(p, normal, tangent, glm::vec2(c, s));
        meshData.vertices[circleStart2 + i] = Vertex(p, glm::vec3(0.f, 0.f, -1.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec2(c, s));
    }

    // Define indices
    // Cone/Arrow upper mesh
    GLuint vi = 0;
    for (std::size_t i = 0; i < trianglesNum / 2; ++i)
    {
        meshData.indices[i * 3] = vi;
        meshData.indices[i * 3 + 1] = circleStart + vi;
        vi = (vi + 1) >= circleStart ? 0 : (vi + 1);
        meshData.indices[i * 3 + 2] = circleStart + vi;
    }

    // Circle
    vi = 0;
    for (std::size_t i = trianglesNum / 2; i < trianglesNum; ++i)
    {
        meshData.indices[i * 3] = vertexNum - 1;
        meshData.indices[i * 3 + 2] = circleStart2 + vi;
        vi = (vi + 1) >= circleStart2 ? 0 : (vi + 1);
        meshData.indices[i * 3 + 1] = circleStart2 + vi;
    }

    meshData.indexed = true;
    meshData.renderMode = GL_TRIANGLES;

    return meshData;
}

MeshData GeometryGenerator::createCylinder()
{
    // Create a simple cylinder that "points" towards z axis

    MeshData meshData;

    const GLuint circleVertexNum = 25;
    const GLuint vertexNum = circleVertexNum * 4 + 2;
    meshData.vertices.resize(vertexNum);
    const GLuint trianglesNum = (circleVertexNum - 1) * 2 + circleVertexNum * 2;
    meshData.indices.resize(trianglesNum * 3);

    // lower and upper circle centers
    meshData.vertices[vertexNum - 2] = Vertex(0, 0, 0, 0, 0, -1, 0, 1, 0, 0.5f, 0.5f);
    meshData.vertices[vertexNum - 1] = Vertex(0, 0, 1, 0, 0, 1, 0, -1, 0, 0.5f, 0.5f);

    float angleStep = math::PI2 / (circleVertexNum - 1);

    GLuint lowerCircleStart = 0;
    GLuint upperCircleStart = lowerCircleStart + circleVertexNum;
    GLuint lowerBaseStart = upperCircleStart + circleVertexNum;
    GLuint upperBaseStart = lowerBaseStart + circleVertexNum;

    for (std::size_t i = 0; i < circleVertexNum; ++i)
    {
        float angle = angleStep * i;
        float c = cosf(angle);
        float s = sinf(angle);

        glm::vec3 p(c, s, 0.0f);
        glm::vec3 tangent = glm::vec3(0, 0, 1);

        meshData.vertices[lowerCircleStart + i] = Vertex(p, glm::vec3(0.f, 0.f, -1.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec2(c, s));
        meshData.vertices[upperCircleStart + i] = Vertex(p + tangent, glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 0.f), glm::vec2(c, s));
        meshData.vertices[lowerBaseStart + i] = Vertex(p, p, tangent, glm::vec2(c, s));
        meshData.vertices[upperBaseStart + i] = Vertex(p + tangent, p, glm::vec3(0, 0, 1), glm::vec2(c, s));
    }

    // Define indices

    // Lower Circle
    GLuint vi = 0;
    for (std::size_t i = 0; i < circleVertexNum - 1; ++i)
    {
        meshData.indices[i * 3] = vertexNum - 2;
        meshData.indices[i * 3 + 2] = lowerCircleStart + vi;
        vi = (vi + 1) >= circleVertexNum ? 0 : (vi + 1);
        meshData.indices[i * 3 + 1] = lowerCircleStart + vi;
    }

    // Upper circle
    vi = 0;
    for (std::size_t i = circleVertexNum - 1; i < (circleVertexNum - 1) * 2; ++i)
    {
        meshData.indices[i * 3] = vertexNum - 1;
        meshData.indices[i * 3 + 2] = upperCircleStart + vi;
        vi = (vi + 1) >= circleVertexNum ? 0 : (vi + 1);
        meshData.indices[i * 3 + 1] = upperCircleStart + vi;
    }

    // Base
    vi = 0;
    for (std::size_t i = (circleVertexNum - 1) * 2; i < trianglesNum; i += 2)
    {
        GLuint v0 = upperBaseStart + vi;
        GLuint v1 = lowerBaseStart + vi;
        vi = (vi + 1) >= circleVertexNum + 1 ? 0 : (vi + 1);
        GLuint v2 = lowerBaseStart + vi;
        GLuint v3 = upperBaseStart + vi;

        meshData.indices[i * 3] = v0;
        meshData.indices[i * 3 + 1] = v1;
        meshData.indices[i * 3 + 2] = v2;

        meshData.indices[i * 3 + 3] = v0;
        meshData.indices[i * 3 + 4] = v2;
        meshData.indices[i * 3 + 5] = v3;
    }

    meshData.computeBitangets();
    meshData.indexed = true;
    meshData.renderMode = GL_TRIANGLES;

    return meshData;
}

MeshData GeometryGenerator::createSquare()
{
    MeshData meshData;

    meshData.vertices.push_back(Vertex(-0.5f, -0.5f, 0.f));
    meshData.vertices.push_back(Vertex(0.5f, -0.5f, 0.f));
    meshData.vertices.push_back(Vertex(-0.5f, 0.5f, 0.f));
    meshData.vertices.push_back(Vertex(0.5f, 0.5f, 0.f));

    meshData.indexed = false;
    meshData.renderMode = GL_TRIANGLE_STRIP;

    return meshData;
}

MeshData GeometryGenerator::createSprite()
{
    MeshData meshData;

    meshData.vertices.push_back(Vertex(0.f, 0.f, 0.f));
    meshData.vertices.push_back(Vertex(1.f, 0.f, 0.f));
    meshData.vertices.push_back(Vertex(0.f, 1.f, 0.f));
    meshData.vertices.push_back(Vertex(1.f, 1.f, 0.f));

    meshData.indexed = false;
    meshData.renderMode = GL_TRIANGLE_STRIP;

    return meshData;
}

MeshData GeometryGenerator::createColoredSquare(float r, float g, float b, float a)
{
    MeshData meshData;

    meshData.vertices.push_back(Vertex(-0.5f, -0.5f, 0.f, r, g, b, a));
    meshData.vertices.push_back(Vertex(0.5f, -0.5f, 0.f, r, g, b, a));
    meshData.vertices.push_back(Vertex(-0.5f, 0.5f, 0.f, r, g, b, a));
    meshData.vertices.push_back(Vertex(0.5f, 0.5f, 0.f, r, g, b, a));

    meshData.indexed = false;
    meshData.renderMode = GL_TRIANGLE_STRIP;

    return meshData;
}
