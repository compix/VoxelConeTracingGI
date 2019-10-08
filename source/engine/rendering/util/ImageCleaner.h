#pragma once
#include <GL/glew.h>
#include <memory>
#include <engine/rendering/shader/Shader.h>

class BBox;

class ImageCleaner
{
public:
    static void init();

    static void clearImage3D(GLuint texID, GLenum format, glm::ivec3 start, glm::ivec3 extent, GLuint resolution, GLuint clipmapLevel, int borderWidth = 0);

    static void clear6FacesImage3D(GLuint texID, GLenum format, glm::ivec3 start, glm::ivec3 extent, GLuint resolution, GLuint clipmapLevel, int borderWidth = 0);

private:
    static void clear(Shader* shader, GLuint texID, glm::ivec3 start, glm::ivec3 extent, GLuint resolution, GLuint clipmapLevel, GLenum format, int borderWidth);
private:
    static std::shared_ptr<Shader> m_clear6FacesClipmapImage3DShader;
    static std::shared_ptr<Shader> m_clearClipmapImage3DShader;
};
