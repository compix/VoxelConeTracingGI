#include "SimpleMeshRenderer.h"
#include <engine/rendering/geometry/MeshBuilder.h>

SimpleMeshRenderer::SimpleMeshRenderer(MeshBuilder& meshBuilder, GLenum renderMode)
{
    m_vbos = meshBuilder.getVBOs();
    m_ibo = meshBuilder.getIBO();
    m_vao = meshBuilder.getVAO();
    m_indexCount = meshBuilder.getIndexCount();
    m_vertexCount = meshBuilder.getVertexCount();
    m_renderMode = renderMode;
    m_indexType = meshBuilder.getIndexType();
}

SimpleMeshRenderer::~SimpleMeshRenderer()
{
    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
    }

    for (auto& vbo : m_vbos)
    {
        glDeleteBuffers(1, &vbo);
    }

    if (m_ibo != 0)
    {
        glDeleteBuffers(1, &m_ibo);
    }
}

void SimpleMeshRenderer::bind() const
{
    glBindVertexArray(m_vao);

    if (m_indexCount > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    }
}

void SimpleMeshRenderer::renderInstanced(GLsizei instanceCount) const
{
    if (m_indexCount > 0)
    {
        glDrawElementsInstanced(m_renderMode, GLsizei(m_indexCount), m_indexType, nullptr, instanceCount);
    }
    else
    {
        glDrawArraysInstanced(m_renderMode, 0, GLsizei(m_vertexCount), instanceCount);
    }
}

void SimpleMeshRenderer::render() const
{
    if (m_indexCount > 0)
    {
        glDrawElements(m_renderMode, GLsizei(m_indexCount), m_indexType, nullptr);
    }
    else
    {
        glDrawArrays(m_renderMode, 0, GLsizei(m_vertexCount));
    }
}

void SimpleMeshRenderer::bindAndRender() const
{
    bind();
    render();
}

void SimpleMeshRenderer::bindAndRenderInstanced(GLsizei instanceCount) const
{
    bind();
    renderInstanced(instanceCount);
}
