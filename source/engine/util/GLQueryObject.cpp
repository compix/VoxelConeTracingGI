#include "GLQueryObject.h"

GLQueryObject::GLQueryObject()
{
    glGenQueries(1, &m_id);
}

GLQueryObject::~GLQueryObject()
{
    glDeleteQueries(1, &m_id);
}

void GLQueryObject::begin(GLenum target)
{
    m_curTarget = target;
    glBeginQuery(target, m_id);
}

void GLQueryObject::end() const
{
    glEndQuery(m_curTarget);
}

GLint64 GLQueryObject::getInt64(GLenum pname) const
{
    GLint64 result = 0;
    glGetQueryObjecti64v(m_id, pname, &result);

    return result;
}

GLuint64 GLQueryObject::getUInt64(GLenum pname) const
{
    GLuint64 result = 0;
    glGetQueryObjectui64v(m_id, pname, &result);

    return result;
}

GLint GLQueryObject::getInt(GLenum pname) const
{
    GLint result = 0;
    glGetQueryObjectiv(m_id, pname, &result);

    return result;
}

GLuint GLQueryObject::getUInt(GLenum pname) const
{
    GLuint result = 0;
    glGetQueryObjectuiv(m_id, pname, &result);

    return result;
}
