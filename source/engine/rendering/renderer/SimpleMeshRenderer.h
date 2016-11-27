#pragma once
#include <GL/glew.h>
#include <vector>

class MeshBuilder;

class SimpleMeshRenderer
{
public:
    SimpleMeshRenderer(MeshBuilder& meshBuilder, GLenum renderMode = GL_TRIANGLES);
    ~SimpleMeshRenderer();

    void bind() const;

    void renderInstanced(GLsizei instanceCount) const;
    void render() const;

    void bindAndRender() const;
    void bindAndRenderInstanced(GLsizei instanceCount) const;

    GLuint getVBO(size_t idx) const { return m_vbos[idx]; }

private:
    std::vector<GLuint> m_vbos;
    GLuint m_ibo{0};
    GLuint m_vao{0};

    size_t m_indexCount{0};
    size_t m_vertexCount{0};
    GLenum m_renderMode{0};
    GLenum m_indexType{0};
};
