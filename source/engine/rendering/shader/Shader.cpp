#include "Shader.h"
#include <GL/glew.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <engine/util/Logger.h>
#include <engine/util/file.h>
#include <engine/rendering/util/GLUtil.h>
#include <fstream>
#include <regex>
#include <engine/resource/ResourceManager.h>

void Shader::shaderErrorCheck(GLuint shader, const file::ShaderSourceInfo& shaderInfo)
{
    GLint result = GL_FALSE;
    GLint infoLogLength;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> shaderLogVector;

    std::string shaderLog;

    if (infoLogLength > 0)
    {
        shaderLogVector.resize(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, nullptr, &shaderLogVector[0]);
        shaderLog = &shaderLogVector[0];
    }

    if (result == GL_FALSE)
        std::cout << "Shader comilation failed for " << shaderInfo.path << std::endl;

    // NVIDIA includes the shader warnings + errors in the program log and supports
    // GL_ARB_shading_language_include -> no need to do anything in this case
    if (shaderLogVector.size() > 0 && glewIsSupported("GL_ARB_shading_language_include") == GL_FALSE)
    {
        // Show include paths, warnings and errors with fixed line numbers relative to the included source code
        std::cout << shaderInfo.path << ": \n";

        // Assuming AMD error/warning logs here
        auto* curInfo = &shaderInfo;
        std::regex r("(\\d+)(:)(\\d+)");
        std::smatch m;

        std::istringstream stream(shaderLog);

        std::string line;
        while (std::getline(stream, line))
        {
            if (std::regex_search(line, m, r))
            {
                int lineNumber = stoi(m.str(3));
                auto& info = shaderInfo.getInfo(lineNumber);
                if (&info != curInfo)
                {
                    curInfo = &info;
                    std::cout << "\n" << info.path << ":\n";
                }
                std::cout << std::regex_replace(line, r, "$1:" +
                    std::to_string(info.getRelativeLineNumber(lineNumber))) << "\n";
            }
            else
                std::cout << line << "\n";
        }
    }
}

void Shader::programErrorCheck(GLuint program, std::initializer_list<std::string> shaderPaths)
{
    GLint result = GL_FALSE;
    GLint infoLogLength;

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> programLog;
    if (infoLogLength > 0)
    {
        programLog.resize(infoLogLength);
        glGetProgramInfoLog(program, infoLogLength, nullptr, &programLog[0]);
    }

    if (result == GL_FALSE)
    {
        std::cout << "Linking the program failed for ";
        for (auto& shaderPath : shaderPaths)
            std::cout << shaderPath << " ";
        std::cout << "\n";
    }

    if (programLog.size() > 0)
    {
        std::cout << "Program Log - ";
        for (auto& shaderPath : shaderPaths)
            std::cout << shaderPath << " ";
        std::cout << std::endl;
        fprintf(stdout, "%s\n", &programLog[0]);
    }
}

void Shader::load(const std::string& vsPath, const std::string& fsPath, std::initializer_list<std::string> vertexAttributeNames)
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);

    m_fsCompPath = fsPath;
    m_vsPath = vsPath;
    m_gsPath = "";
    m_vertexAttributeNames = vertexAttributeNames;

    GLuint vsID = load(GL_VERTEX_SHADER, vsPath);
    GLuint fsID = load(GL_FRAGMENT_SHADER, fsPath);

    GLuint program = glCreateProgram();

    glAttachShader(program, vsID);
    glAttachShader(program, fsID);

    uint8_t i = 0;
    for (auto& n : vertexAttributeNames)
    {
        glBindAttribLocation(program, GLuint(i), n.c_str());
        ++i;
    }

    glLinkProgram(program);

    programErrorCheck(program, { vsPath, fsPath });

    glDeleteShader(vsID);
    glDeleteShader(fsID);

    GL_ERROR_CHECK();

    m_shaderProgram = program;
    m_loadedProgram = true;
}

void Shader::loadCompute(const std::string& path)
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);

    m_fsCompPath = path;
    m_vsPath = "";
    m_gsPath = "";

    GLuint id = load(GL_COMPUTE_SHADER, path);

    GLuint program = glCreateProgram();

    glAttachShader(program, id);

    glLinkProgram(program);

    programErrorCheck(program, { path });

    glDeleteShader(id);

    GL_ERROR_CHECK();

    m_shaderProgram = program;
    m_loadedProgram = true;
}

Shader::Shader(const std::string& vsPath, const std::string& fsPath)
{
    load(vsPath, fsPath);
}

Shader::~Shader()
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);
}

void Shader::load(const std::string& vsPath, const std::string& fsPath, const std::string& gsPath)
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);

    m_fsCompPath = fsPath;
    m_vsPath = vsPath;
    m_gsPath = gsPath;

    GLuint vsID = load(GL_VERTEX_SHADER, vsPath);
    GLuint fsID = load(GL_FRAGMENT_SHADER, fsPath);
    GLuint gsID = load(GL_GEOMETRY_SHADER, gsPath);

    GLuint program = glCreateProgram();

    glAttachShader(program, vsID);
    glAttachShader(program, fsID);
    glAttachShader(program, gsID);

    glLinkProgram(program);

    programErrorCheck(program, { vsPath, fsPath, gsPath });

    glDeleteShader(vsID);
    glDeleteShader(fsID);
    glDeleteShader(gsID);

    GL_ERROR_CHECK();

    m_shaderProgram = program;
    m_loadedProgram = true;
}

void Shader::bindTexture2D(GLuint texId, const std::string& textureName, GLint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texId);
    glUniform1i(glGetUniformLocation(m_shaderProgram, textureName.c_str()), textureUnit);
}

void Shader::bindTexture3D(GLuint texId, const std::string& textureName, GLint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_3D, texId);
    glUniform1i(glGetUniformLocation(m_shaderProgram, textureName.c_str()), textureUnit);
}

void Shader::bindImage2D(GLuint texId, const std::string& textureName, GLenum access, GLenum format, GLint textureUnit, GLint level)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindImageTexture(GLuint(textureUnit), texId, level, GL_FALSE, 0, access, format);
    glUniform1i(glGetUniformLocation(m_shaderProgram, textureName.c_str()), textureUnit);
}

void Shader::bindImage3D(GLuint texId, const std::string& textureName, GLenum access, GLenum format, GLint textureUnit, GLint level)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindImageTexture(GLuint(textureUnit), texId, level, GL_TRUE, 0, access, format);
    glUniform1i(glGetUniformLocation(m_shaderProgram, textureName.c_str()), textureUnit);
}

void Shader::dispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ)
{
    assert(GL::isShaderBound(m_shaderProgram));
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void Shader::recompile()
{
    if (!m_loadedProgram)
        return;

    if (m_gsPath == "")
    {
        if (m_vsPath == "")
            loadCompute(m_fsCompPath);
        else
            load(m_vsPath, m_fsCompPath, m_vertexAttributeNames);

        return;
    }

    load(m_vsPath, m_fsCompPath, m_gsPath);
}

void Shader::showComputeShaderLimits()
{
    std::cout << "Compute shader limits: " << std::endl;
    GLint d;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &d);
    std::cout << "Max local_size_(x, y, z): " << d << " ";
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &d);
    std::cout << d << " ";
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &d);
    std::cout << d << "\n";

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &d);
    std::cout << "Max dispatch work group count (x, y, z): " << d << " ";
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &d);
    std::cout << d << " ";
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &d);
    std::cout << d << "\n";

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &d);
    std::cout << "Max local_size product (x * y * z): " << d << "\n";

    glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &d);
    std::cout << "Max shared memory size: " << (d / 1024) << "KB \n\n";
}

GLuint Shader::load(GLenum shaderType, const std::string& shaderPath)
{
    GLuint id = glCreateShader(shaderType);

    //std::string source = file::readAsString(shaderPath);

    // Read in file as string and resolve #includes
    auto sourceInfo = file::getShaderSource(shaderPath);

    char const* cSource = sourceInfo.source.c_str();
    glShaderSource(id, 1, &cSource, nullptr);
    glCompileShader(id);

    shaderErrorCheck(id, sourceInfo);

    return id;
}

void Shader::bind()
{
    glUseProgram(m_shaderProgram);
}

void Shader::setCamera(const glm::mat4& view, const glm::mat4& proj) const noexcept
{
    glUniformMatrix4fv(getLocation("u_view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(getLocation("u_proj"), 1, GL_FALSE, &proj[0][0]);
}

void Shader::setEyePosition(const glm::vec3& pos) { setVector("u_eyePos", pos); }

void Shader::setModel(const glm::mat4& modelMatrix, bool setInverseTranspose) const noexcept
{
    glUniformMatrix4fv(getLocation("u_model"), 1, GL_FALSE, &modelMatrix[0][0]);

    if (setInverseTranspose)
    {
        glm::mat4 modelIT = glm::transpose(glm::inverse(modelMatrix));
        glUniformMatrix4fv(getLocation("u_modelIT"), 1, GL_FALSE, &modelIT[0][0]);
    }
}

void Shader::setColor(float r, float g, float b, float a) const noexcept
{
    glUniform4f(getLocation("u_color"), r, g, b, a);
}

void Shader::setColor(glm::vec4 color) const noexcept
{
    glUniform4f(getLocation("u_color"), color.r, color.g, color.b, color.a);
}

void Shader::setColor(glm::vec3 color) const noexcept { setColor(color.r, color.g, color.b); }

void Shader::setColor(float r, float g, float b) const noexcept
{
    glUniform3f(getLocation("u_color"), r, g, b);
}

void Shader::setMVP(const glm::mat4& mvp) const noexcept
{
    glUniformMatrix4fv(getLocation("u_modelViewProj"), 1, GL_FALSE, &mvp[0][0]);
}

void Shader::setFloat(const UniformName& uniformName, float v) const noexcept
{
    glUniform1f(getLocation(uniformName.c_str()), v);
}

void Shader::setDouble(const UniformName& uniformName, double v) const noexcept
{
    glUniform1d(getLocation(uniformName.c_str()), v);
}

void Shader::setInt(const UniformName& uniformName, int v) const noexcept
{
    glUniform1i(getLocation(uniformName.c_str()), v);
}

void Shader::setUnsignedInt(const UniformName& uniformName, glm::uint v) const noexcept
{
    glUniform1ui(getLocation(uniformName.c_str()), v);
}

void Shader::setVector(const UniformName& uniformName, float v1, float v2) const noexcept
{
    glUniform2f(getLocation(uniformName.c_str()), v1, v2);
}

void Shader::setVector(const UniformName& uniformName, float v1, float v2, float v3) const noexcept
{
    glUniform3f(getLocation(uniformName.c_str()), v1, v2, v3);
}

void Shader::setVector(const UniformName& uniformName, float v1, float v2, float v3, float v4) const noexcept
{
    glUniform4f(getLocation(uniformName.c_str()), v1, v2, v3, v4);
}

void Shader::setVector(const UniformName& uniformName, const glm::vec2& v) const noexcept
{
    glUniform2f(getLocation(uniformName.c_str()), v.x, v.y);
}

void Shader::setVector(const UniformName& uniformName, const glm::vec3& v) const noexcept
{
    glUniform3f(getLocation(uniformName.c_str()), v.x, v.y, v.z);
}

void Shader::setVector(const UniformName& uniformName, const glm::vec4& v) const noexcept
{
    glUniform4f(getLocation(uniformName.c_str()), v.x, v.y, v.z, v.w);
}

void Shader::setMatrix(const UniformName& uniformName, const glm::mat4& m) const noexcept
{
    glUniformMatrix4fv(getLocation(uniformName.c_str()), 1, GL_FALSE, &m[0][0]);
}

void Shader::setMatrix(const UniformName& uniformName, const glm::mat3& m) const noexcept
{
    glUniformMatrix3fv(getLocation(uniformName.c_str()), 1, GL_FALSE, &m[0][0]);
}

void Shader::setMatrix(const UniformName& uniformName, const glm::mat2& m) const noexcept
{
    glUniformMatrix2fv(getLocation(uniformName.c_str()), 1, GL_FALSE, &m[0][0]);
}

void Shader::setVectori(const UniformName& uniformName, const glm::ivec2& v) const noexcept
{
    glUniform2i(getLocation(uniformName.c_str()), v.x, v.y);
}

void Shader::setVectori(const UniformName& uniformName, const glm::ivec3& v) const noexcept
{
    glUniform3i(getLocation(uniformName.c_str()), v.x, v.y, v.z);
}

void Shader::setVectori(const UniformName& uniformName, const glm::ivec4& v) const noexcept
{
    glUniform4i(getLocation(uniformName.c_str()), v.x, v.y, v.z, v.w);
}
