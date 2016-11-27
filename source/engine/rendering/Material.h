#pragma once
#include <string>
#include <unordered_map>
#include <glm/detail/type_vec3.hpp>
#include <memory>
#include "shader/Shader.h"
#include "Texture2D.h"

using TextureName = std::string;

struct EditableMaterialDesc
{
    EditableMaterialDesc() {}
    EditableMaterialDesc(float min, float max)
        :min(min), max(max) {}
    EditableMaterialDesc(bool isColor)
        :isColor(isColor) {}

    float min{0.0f};
    float max{0.0f};
    bool isColor{ false };
};

class EditableMaterialProperties
{
public:
    static void init();

    static const EditableMaterialDesc* getDesc(const UniformName& uniformName)
    {
        auto it = m_materialDescs.find(uniformName);
        if (it != m_materialDescs.end())
            return &it->second;

        return nullptr;
    }

private:

    static std::unordered_map<UniformName, EditableMaterialDesc> m_materialDescs;
};

class Material
{
    friend class MeshRenderer;
    friend class MeshRenderSystem;
public:
    Material() { }

    explicit Material(std::shared_ptr<Shader> shader);

    void setShader(std::shared_ptr<Shader> shader);

    void setTexture2D(const TextureName& textureName, TextureID textureID);
    void setTexture3D(const TextureName& textureName, TextureID textureID);

    std::shared_ptr<Shader> getShader() const { return m_shader; }

    void setFloat(const UniformName& uniformName, float v) noexcept { m_floatMap[uniformName] = v; }

    void setVector(const UniformName& uniformName, const glm::vec2& v) noexcept { m_vec2Map[uniformName] = v; }

    void setVector(const UniformName& uniformName, const glm::vec3& v) noexcept { m_vec3Map[uniformName] = v; }

    void setVector(const UniformName& uniformName, const glm::vec4& v) noexcept { m_vec4Map[uniformName] = v; }

    void setColor(const UniformName& uniformName, const glm::vec3& v) noexcept { m_vec3Map[uniformName] = v; }

    void setColor(const UniformName& uniformName, const glm::vec4& v) noexcept { m_vec4Map[uniformName] = v; }

    void setMatrix(const UniformName& uniformName, const glm::mat2& m) noexcept { m_mat2Map[uniformName] = m; }

    void setMatrix(const UniformName& uniformName, const glm::mat3& m) noexcept { m_mat3Map[uniformName] = m; }

    void setMatrix(const UniformName& uniformName, const glm::mat4& m) noexcept { m_mat4Map[uniformName] = m; }

private:
    void use();
    void use(Shader* shader, bool bind = false);

private:
    std::shared_ptr<Shader> m_shader;
    std::unordered_map<TextureName, TextureID> m_textures2D;
    std::unordered_map<TextureName, TextureID> m_textures3D;

    std::unordered_map<UniformName, float> m_floatMap;
    std::unordered_map<UniformName, glm::vec2> m_vec2Map;
    std::unordered_map<UniformName, glm::vec3> m_vec3Map;
    std::unordered_map<UniformName, glm::vec4> m_vec4Map;
    std::unordered_map<UniformName, glm::mat2> m_mat2Map;
    std::unordered_map<UniformName, glm::mat3> m_mat3Map;
    std::unordered_map<UniformName, glm::mat4> m_mat4Map;
};
