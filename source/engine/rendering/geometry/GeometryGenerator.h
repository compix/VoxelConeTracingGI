#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

/**
* @brief Generic vertex.
*/
struct Vertex
{
    Vertex() { }

    Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec3& t, const glm::vec2& uv)
        : pos(p), normal(n), tangent(t), uv(uv) { }

    Vertex(
        float px, float py, float pz,
        float nx, float ny, float nz,
        float tx, float ty, float tz,
        float u, float v)
        : pos(px, py, pz), normal(nx, ny, nz),
          tangent(tx, ty, tz), uv(u, v) { }

    Vertex(const glm::vec3& pos) : pos(pos) { }

    Vertex(float px, float py, float pz) : pos(px, py, pz) { }

    Vertex(float px, float py, float pz, float r, float g, float b, float a)
        : pos(px, py, pz), color(r, g, b, a) { }

    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitanget;
    glm::vec2 uv;
    glm::vec4 color;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    bool indexed;
    GLenum renderMode;

    void computeBitangets();
};

class GeometryGenerator
{
public:
    static MeshData createBox(float width, float height, float depth);
    static MeshData createArrowHead();
    static MeshData createCylinder();
    static MeshData createSphere(float radius, GLuint sliceCount, GLuint stackCount);
    static MeshData createSquare();
    static MeshData createSprite();
    static MeshData createColoredSquare(float r, float g, float b, float a);
};
