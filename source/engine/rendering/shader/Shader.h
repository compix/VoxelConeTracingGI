#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace file {
    struct ShaderSourceInfo;
}

using ShaderProgram = GLuint;
using UniformName = std::string;

class Shader
{
public:
    Shader() {}

    Shader(const std::string& vsPath, const std::string& fsPath);
    ~Shader();

    void load(const std::string& vsPath, const std::string& fsPath, const std::string& gsPath);
    void load(const std::string& vsPath, const std::string& fsPath, std::initializer_list<std::string> vertexAttributeNames = {});
    void loadCompute(const std::string& path);

    ShaderProgram getProgram() const noexcept { return m_shaderProgram; }

    void bind();

    /**
    * Sets the camera view and proj matrices "u_view" and "u_proj".
    */
    void setCamera(const glm::mat4& view, const glm::mat4& proj) const noexcept;

    /**
    * Sets the eye/camera position "u_eyePos".
    */
    void setEyePosition(const glm::vec3& pos);

    /**
    * Sets the model matrix "u_model".
    * If setInverseTranspose is true then "u_modelIT"
    * will be set as the inverse transpose of the given model matrix.
    */
    void setModel(const glm::mat4& modelMatrix, bool setInverseTranspose = true) const noexcept;

    /**
    * Sets the 4 component color "u_color".
    */
    void setColor(float r, float g, float b, float a) const noexcept;

    /**
    * Sets the 4 component color "u_color".
    */
    void setColor(glm::vec4 color) const noexcept;

    /**
    * Sets the 3 component color "u_color".
    */
    void setColor(glm::vec3 color) const noexcept;

    /**
    * Sets the 3 component color "u_color".
    */
    void setColor(float r, float g, float b) const noexcept;

    /**
    * Sets the model view projection matrix "u_modelViewProj".
    */
    void setMVP(const glm::mat4& mvp) const noexcept;

    void setShaderProgram(ShaderProgram shaderProgram) noexcept { m_shaderProgram = shaderProgram; }

    bool hasSameProgram(ShaderProgram shaderProgram) const noexcept { return m_shaderProgram == shaderProgram; }

    void setFloat(const UniformName& uniformName, float v) const noexcept;
    void setDouble(const UniformName& uniformName, double v) const noexcept;
    void setInt(const UniformName& uniformName, int v) const noexcept;
    void setUnsignedInt(const UniformName& uniformName, glm::uint v) const noexcept;
    void setVector(const UniformName& uniformName, float v1, float v2) const noexcept;
    void setVector(const UniformName& uniformName, float v1, float v2, float v3) const noexcept;
    void setVector(const UniformName& uniformName, float v1, float v2, float v3, float v4) const noexcept;
    void setVector(const UniformName& uniformName, const glm::vec2& v) const noexcept;
    void setVector(const UniformName& uniformName, const glm::vec3& v) const noexcept;
    void setVector(const UniformName& uniformName, const glm::vec4& v) const noexcept;
    void setMatrix(const UniformName& uniformName, const glm::mat4& m) const noexcept;
    void setMatrix(const UniformName& uniformName, const glm::mat3& m) const noexcept;
    void setMatrix(const UniformName& uniformName, const glm::mat2& m) const noexcept;

    void setVectori(const UniformName& uniformName, const glm::ivec2& v) const noexcept;
    void setVectori(const UniformName& uniformName, const glm::ivec3& v) const noexcept;
    void setVectori(const UniformName& uniformName, const glm::ivec4& v) const noexcept;

    /**
    * textureName is the name of the texture in the shader.
    * textureUnit: Use 0 for the first sampler/texture, 1 for the second ... The order of samplers/textures in the shader doesn't matter.
    */
    void bindTexture2D(GLuint texId, const std::string& textureName, GLint textureUnit = 0);

    /**
    * textureName is the name of the texture in the shader.
    * textureUnit: Use 0 for the first sampler/texture, 1 for the second ... The order of samplers/textures in the shader doesn't matter.
    */
    void bindTexture3D(GLuint texId, const std::string& textureName, GLint textureUnit = 0);

    void bindImage2D(GLuint texId, const std::string& textureName, GLenum access, GLenum format, GLint textureUnit = 0, GLint level = 0);
    void bindImage3D(GLuint texId, const std::string& textureName, GLenum access, GLenum format, GLint textureUnit = 0, GLint level = 0);

    void dispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ = 1);

    void recompile();

    static void showComputeShaderLimits();

    GLint getLocation(const char* uniformName) const noexcept { return glGetUniformLocation(m_shaderProgram, uniformName); }

private:
    GLuint load(GLenum shaderType, const std::string& shaderPath);
    void shaderErrorCheck(GLuint shader, const file::ShaderSourceInfo& shaderInfo);
    void programErrorCheck(GLuint program, std::initializer_list<std::string> shaderPaths);

private:
    ShaderProgram m_shaderProgram = 0;
    bool m_loadedProgram = false;

    // Is either the fragment shader path or the compute shader path
    std::string m_fsCompPath;
    std::string m_vsPath;
    std::string m_gsPath;
    std::initializer_list<std::string> m_vertexAttributeNames;
};
