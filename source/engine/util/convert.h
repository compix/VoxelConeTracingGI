#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <cstddef>

namespace convert
{
    template <class T>
    GLenum toGLEnum();

    template <>
    constexpr GLenum toGLEnum<float>() { return GL_FLOAT; }

    template <>
    constexpr GLenum toGLEnum<int8_t>() { return GL_BYTE; }

    template <>
    constexpr GLenum toGLEnum<int16_t>() { return GL_SHORT; }

    template <>
    constexpr GLenum toGLEnum<int32_t>() { return GL_INT; }

    template <>
    constexpr GLenum toGLEnum<uint8_t>() { return GL_UNSIGNED_BYTE; }

    template <>
    constexpr GLenum toGLEnum<uint16_t>() { return GL_UNSIGNED_SHORT; }

    template <>
    constexpr GLenum toGLEnum<uint32_t>() { return GL_UNSIGNED_INT; }

    std::size_t sizeFromGLType(GLenum type);
}
