#pragma once

#include <sstream>
#include <iostream>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <GL/glew.h>
#include <GL/glu.h>

class Logger
{
public:
    Logger();

    static void log(const std::string& msg) noexcept;

    static std::ostream& stream() noexcept { return std::cout; }

    static std::ostream& errorStream() noexcept { return std::cerr; }
};

inline std::ostream& operator <<(std::ostream& os, const glm::mat4& m)
{
    return os << m[0][0] << ", " << m[1][0] << ", " << m[2][0] << ", " << m[3][0] << std::endl
        << m[0][1] << ", " << m[1][1] << ", " << m[2][1] << ", " << m[3][1] << std::endl
        << m[0][2] << ", " << m[1][2] << ", " << m[2][2] << ", " << m[3][2] << std::endl
        << m[0][3] << ", " << m[1][3] << ", " << m[2][3] << ", " << m[3][3];
}

inline std::ostream& operator <<(std::ostream& os, const glm::vec2& v) { return os << "(" << v.x << ", " << v.y << ")"; }

inline std::ostream& operator <<(std::ostream& os, const glm::vec3& v) { return os << "(" << v.x << ", " << v.y << ", " << v.z << ")"; }

inline std::ostream& operator <<(std::ostream& os, const glm::vec4& v) { return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")"; }

inline std::ostream& operator <<(std::ostream& os, const glm::quat& q) { return os << "(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")"; }

#define LOG_INFO_ATTACHMENT ":\nFUNCTION: " << __func__ << "\nFILE: " << __FILE__ << "\nLINE: " << __LINE__
#define LOG_ERROR(M) do {Logger::errorStream() << M << LOG_INFO_ATTACHMENT << "\n\n";} while(0)
#define LOG_EXIT(M) do {Logger::errorStream() << M << LOG_INFO_ATTACHMENT << "\n\n"; exit(1);} while(0)
#define LOG(M) do {Logger::stream() << M << "\n";} while(0)

#define SDL_ERROR_CHECK() do            \
{                                       \
    const char* error = SDL_GetError(); \
    if (*error != '\0')                 \
        LOG_ERROR("SDL: " << error);    \
    SDL_ClearError();                   \
} while (0)

#define GL_ERROR_CHECK() do {                     \
    GLenum errCode;                               \
    const GLubyte *errString;                     \
    if ((errCode = glGetError()) != GL_NO_ERROR)  \
    {                                             \
        errString = gluErrorString(errCode);      \
        LOG_ERROR("OpenGL Error: " << errString); \
    }                                             \
} while (0)
