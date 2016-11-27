#include "MeshBuilder.h"
#include <assert.h>
#include <engine/util/Logger.h>
#include <engine/util/convert.h>

VBODescription::VBODescription(size_t size, const void* data, GLenum usage)
    : m_size(size), m_data(data), m_usage(usage) {}

VBODescription& VBODescription::attribute(GLint size, GLenum type, GLuint attribDivisor, GLboolean normalized)
{
    // Cannot deduce offset if user provides custom offsets
    // so make sure it doesn't happen
    assert(m_deducedOffset);

    // Queue attributes to deduce stride
    m_attributes.push_back(Attr(size, type, attribDivisor, normalized));
    return *this;
}

VBODescription& VBODescription::attribute(GLint size, GLenum type, const void* offset, GLsizei stride, GLuint attribDivisor, GLboolean normalized)
{
    m_deducedOffset = false;
    m_attributes.push_back(Attr(size, type, attribDivisor, normalized, offset, stride));
    return *this;
}

MeshBuilder::MeshBuilder(size_t vertexCount)
    : m_vertexCount(vertexCount) {}

void MeshBuilder::reset()
{
    *this = MeshBuilder(m_vertexCount);
}

MeshBuilder& MeshBuilder::createVBO(VBODescription& vboDescription)
{
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vboDescription.m_size, vboDescription.m_data, vboDescription.m_usage);
    m_vbos.push_back(vbo);
    m_vboDescriptions.push_back(vboDescription);
    GL_ERROR_CHECK();
    return *this;
}

MeshBuilder& MeshBuilder::createIBO(size_t indexCount, const void* data, GLenum indexType, GLenum usage)
{
    m_indexType = indexType;
    m_indexCount = indexCount;
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * convert::sizeFromGLType(indexType), data, usage);

    GL_ERROR_CHECK();
    return *this;
}

void MeshBuilder::finalize()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    GL_ERROR_CHECK();

    GLuint idx = 0;

    for (size_t i = 0; i < m_vboDescriptions.size(); ++i)
    {
        auto& vboDesc = m_vboDescriptions[i];
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[i]);

        size_t stride = 0;
        if (vboDesc.m_deducedOffset) // Compute stride
            for (auto& attr : vboDesc.m_attributes)
                stride += attr.size * convert::sizeFromGLType(attr.type);

        size_t offset = 0;

        // Bind attributes
        for (auto& attr : vboDesc.m_attributes)
        {
            glEnableVertexAttribArray(idx);
            if (vboDesc.m_deducedOffset)
            glVertexAttribPointer(idx, attr.size, attr.type, attr.normalized, GLsizei(stride), reinterpret_cast<void*>(offset));
            else
            {
                stride = attr.stride;
                glVertexAttribPointer(idx, attr.size, attr.type, attr.normalized, GLsizei(stride), attr.offset);
            }

            if (attr.divisor != 0)
            glVertexAttribDivisor(idx, attr.divisor);

            // Compute offset
            offset += convert::sizeFromGLType(attr.type) * attr.size;
            ++idx;
            GL_ERROR_CHECK();
        }
    }

    // Unbind all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GL_ERROR_CHECK();
}
