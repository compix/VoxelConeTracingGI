#pragma once
#include <GL/glew.h>
#include <vector>
#include <engine/util/Logger.h>
#include <cstddef>

class VBODescription
{
    friend class MeshBuilder;

    /**
    * Used to queue attributes to deduce offset and stride.
    */
    struct Attr
    {
        Attr() : offset(nullptr) { }

        Attr(GLint size, GLenum type, GLuint divisor, GLboolean normalized)
            : size(size), type(type), divisor(divisor), normalized(normalized), offset(nullptr) { }

        Attr(GLint size, GLenum type, GLuint divisor, GLboolean normalized, const void* offset, std::size_t stride)
            : size(size), type(type), divisor(divisor), normalized(normalized), offset(offset), stride(stride) { }

        GLint size{0};
        GLenum type{0};
        GLuint divisor{0};
        GLboolean normalized{false};
        const void* offset;
        std::size_t stride{0};
    };

public:
    /**
    * Creates a vertex buffer object with the given size and a pointer to the data.
    * Usage is defined to be GL_STATIC_DRAW by default.
    */
    VBODescription(std::size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);

    /**
    * Specify the next vertex attribute - offsets and stride will be deduced when finalize() is called.
    * Attributes in the vertex buffer object are expected to be interleaved.
    * Order of attribute() calls should correspond to the order of attributes in the interleaved buffer object.
    */
    VBODescription& attribute(GLint size, GLenum type, GLuint attribDivisor = 0, GLboolean normalized = false);

    /**
    * Specify the next vertex attribute with offset and stride.
    * Order of attribute() calls should correspond to the order of attributes in the buffer object.
    */
    VBODescription& attribute(GLint size, GLenum type, const void* offset, GLsizei stride, GLuint attribDivisor = 0, GLboolean normalized = false);

private:
    bool m_deducedOffset{true};

    std::size_t m_size{0};
    const void* m_data;
    GLenum m_usage;
    std::vector<Attr> m_attributes;
};

class MeshBuilder
{
public:
    explicit MeshBuilder(std::size_t vertexCount);

    void reset();

    /**
    * Creates a vertex buffer object with the given size and a pointer to the data.
    * Usage is defined to be GL_STATIC_DRAW by default.
    */
    MeshBuilder& createVBO(VBODescription& vboDescription);

    /**
    * Creates an index buffer object with the given index count and a pointer to the data.
    * Specify the index type as a template paramter - e.g. createIBO<GLuint>()
    * The type of the indicies is defined to be GL_UNSIGNED_INT by default.
    * Usage is defined to be GL_STATIC_DRAW by default.
    */
    MeshBuilder& createIBO(std::size_t indexCount, const void* data, GLenum indexType = GL_UNSIGNED_INT, GLenum usage = GL_STATIC_DRAW);

    /**
    * Creates a vertex array object and binds attributes.
    */
    void finalize();

    std::vector<GLuint> getVBOs() const { return m_vbos; }

    GLuint getVBO(std::size_t idx) const { return m_vbos[idx]; }

    GLuint getIBO() const { return m_ibo; }

    GLuint getVAO() const { return m_vao; }

    std::size_t getIndexCount() const { return m_indexCount; }

    std::size_t getVertexCount() const { return m_vertexCount; }

    GLenum getIndexType() const { return m_indexType; }

private:
    std::vector<VBODescription> m_vboDescriptions;
    std::vector<GLuint> m_vbos;
    GLuint m_ibo{0};
    GLuint m_vao{0};

    GLenum m_indexType{0};
    std::size_t m_indexCount{0};
    std::size_t m_vertexCount{0};
};
