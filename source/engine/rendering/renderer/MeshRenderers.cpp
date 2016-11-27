#include "MeshRenderers.h"
#include <glm/glm.hpp>
#include <engine/rendering/geometry/MeshBuilder.h>
#include <engine/rendering/geometry/Mesh.h>
#include <engine/resource/ResourceManager.h>

std::shared_ptr<SimpleMeshRenderer> MeshRenderers::m_quad;
std::shared_ptr<SimpleMeshRenderer> MeshRenderers::m_fullscreenQuad;
std::shared_ptr<SimpleMeshRenderer> MeshRenderers::m_line;
std::shared_ptr<SimpleMeshRenderer> MeshRenderers::m_nonFilledBox;
std::shared_ptr<SimpleMeshRenderer> MeshRenderers::m_triangle;

std::shared_ptr<MeshRenderer> MeshRenderers::m_box;
std::shared_ptr<MeshRenderer> MeshRenderers::m_arrowHead;
std::shared_ptr<MeshRenderer> MeshRenderers::m_cylinder;
std::shared_ptr<MeshRenderer> MeshRenderers::m_sphere;

void MeshRenderers::init()
{
    loadQuad();
    loadFullscreenQuad();
    loadLine();
    loadNonFilledCube();
    loadTriangle();

    auto material = std::make_shared<Material>(ResourceManager::getShader("shaders/debug/debug.vert", "shaders/debug/debug.frag", {"in_pos", "in_normal"}));

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createArrowHead(), VERTEX_POS | VERTEX_NORMAL);
    m_arrowHead = std::make_shared<MeshRenderer>(mesh);
    m_arrowHead->addMaterial(material);

    mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createCylinder(), VERTEX_POS | VERTEX_NORMAL);
    m_cylinder = std::make_shared<MeshRenderer>(mesh);
    m_cylinder->addMaterial(material);

    mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createSphere(0.5f, 32, 32), VERTEX_POS | VERTEX_NORMAL);
    m_sphere = std::make_shared<MeshRenderer>(mesh);
    m_sphere->addMaterial(material);

    mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createBox(1.0f, 1.0f, 1.0f), VERTEX_POS | VERTEX_NORMAL | VERTEX_TANGENT | VERTEX_UV);
    m_box = std::make_shared<MeshRenderer>(mesh);
    m_box->addMaterial(material);
}

void MeshRenderers::loadQuad()
{
    std::vector<glm::vec3> vertices;
    vertices.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
    vertices.push_back(glm::vec3(0.5f, -0.5f, 0.f));
    vertices.push_back(glm::vec3(-0.5f, 0.5f, 0.f));
    vertices.push_back(glm::vec3(0.5f, 0.5f, 0.f));

    MeshBuilder builder(vertices.size());
    VBODescription vboDesc(vertices.size() * sizeof(glm::vec3), &vertices[0]);
    vboDesc.attribute(3, GL_FLOAT);

    builder.createVBO(vboDesc).finalize();
    m_quad = std::make_shared<SimpleMeshRenderer>(builder, GL_TRIANGLE_STRIP);
}

void MeshRenderers::loadFullscreenQuad()
{
    std::vector<glm::vec2> vertices;
    vertices.push_back(glm::vec2(-1.0f, -1.0f));
    vertices.push_back(glm::vec2(-1.0f, 1.0f));
    vertices.push_back(glm::vec2(1.0f, -1.0f));
    vertices.push_back(glm::vec2(1.0f, 1.0f));

    MeshBuilder builder(vertices.size());
    VBODescription vboDesc(vertices.size() * sizeof(glm::vec2), &vertices[0]);
    vboDesc.attribute(2, GL_FLOAT);

    builder.createVBO(vboDesc).finalize();
    m_fullscreenQuad = std::make_shared<SimpleMeshRenderer>(builder, GL_TRIANGLE_STRIP);
}

void MeshRenderers::loadLine()
{
    float vertices[2]{0.0f, 1.0f};

    MeshBuilder builder(2);
    VBODescription vboDesc(2 * sizeof(float), &vertices[0]);
    vboDesc.attribute(1, GL_FLOAT);
    builder.createVBO(vboDesc).finalize();
    m_line = std::make_shared<SimpleMeshRenderer>(builder, GL_LINES);
}

void MeshRenderers::loadNonFilledCube()
{
    float vertices[]{
            // Back
            -0.5f, -0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,

            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f, -0.5f,

            // Front
            -0.5f, -0.5f, 0.5f,

            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, 0.5f,

            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,

            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,

            -0.5f, -0.5f, 0.5f,
        };

    MeshBuilder builder(16);
    VBODescription vboDesc(48 * sizeof(float), &vertices[0]);
    vboDesc.attribute(3, GL_FLOAT);
    builder.createVBO(vboDesc).finalize();
    m_nonFilledBox = std::make_shared<SimpleMeshRenderer>(builder, GL_LINE_STRIP);
}

void MeshRenderers::loadTriangle()
{
    float vertices[]{
        0.0f, 1.0f, 2.0f
    };

    MeshBuilder builder(3);
    VBODescription vboDesc(3 * sizeof(float), &vertices[0]);
    vboDesc.attribute(1, GL_FLOAT);
    builder.createVBO(vboDesc).finalize();
    m_triangle = std::make_shared<SimpleMeshRenderer>(builder, GL_TRIANGLE_STRIP);
}
