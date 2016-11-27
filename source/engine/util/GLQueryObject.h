#pragma once
#include <GL/glew.h>

class GLQueryObject
{
public:
    GLQueryObject();
    ~GLQueryObject();

    /**
     * target is for example GL_TIME_ELAPSED - more information can be found in the OpenGL query object specification
     */
    void begin(GLenum target);
    void end() const;

    GLint64 getInt64(GLenum pname = GL_QUERY_RESULT) const;
    GLuint64 getUInt64(GLenum pname = GL_QUERY_RESULT) const;

    GLint getInt(GLenum pname = GL_QUERY_RESULT) const;
    GLuint getUInt(GLenum pname = GL_QUERY_RESULT) const;

    GLuint getID() const { return m_id; }

private:
    GLuint m_id{ 0 };
    GLenum m_curTarget{ 0 };
};
