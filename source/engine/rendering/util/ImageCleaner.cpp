#include "ImageCleaner.h"
#include <engine/resource/ResourceManager.h>

std::shared_ptr<Shader> ImageCleaner::m_clear6FacesClipmapImage3DShader;
std::shared_ptr<Shader> ImageCleaner::m_clearClipmapImage3DShader;

void ImageCleaner::init()
{
    m_clear6FacesClipmapImage3DShader = ResourceManager::getComputeShader("shaders/voxelConeTracing/clear6FacesClipmapImage3D.comp");
    m_clearClipmapImage3DShader = ResourceManager::getComputeShader("shaders/voxelConeTracing/clearClipmapImage3D.comp");
}

void ImageCleaner::clearImage3D(GLuint texID, GLenum format, glm::ivec3 start, glm::ivec3 extent, GLuint resolution, GLuint clipmapLevel, int borderWidth)
{
    clear(m_clearClipmapImage3DShader.get(), texID, start, extent, resolution, clipmapLevel, format, borderWidth);
}

void ImageCleaner::clear6FacesImage3D(GLuint texID, GLenum format, glm::ivec3 start, glm::ivec3 extent, GLuint resolution, GLuint clipmapLevel, int borderWidth)
{
    clear(m_clear6FacesClipmapImage3DShader.get(), texID, start, extent, resolution, clipmapLevel, format, borderWidth);
}

void ImageCleaner::clear(Shader* shader, GLuint texID, glm::ivec3 start, glm::ivec3 extent, GLuint resolution, GLuint clipmapLevel, GLenum format, int borderWidth)
{
    shader->bind();

    shader->setVectori("u_min", start);
    shader->bindImage3D(texID, "u_image", GL_WRITE_ONLY, format);
    shader->setInt("u_resolution", int(resolution));
    shader->setInt("u_clipmapLevel", int(clipmapLevel));
    shader->setVectori("u_extent", extent);
    shader->setInt("u_borderWidth", borderWidth);

    extent = glm::ivec3(glm::ceil(glm::vec3(extent) / 8.0f));
    shader->dispatchCompute(static_cast<GLuint>(extent.x), static_cast<GLuint>(extent.y), static_cast<GLuint>(extent.z));
}
