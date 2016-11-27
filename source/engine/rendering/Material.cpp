#include "Material.h"

std::unordered_map<UniformName, EditableMaterialDesc> EditableMaterialProperties::m_materialDescs;

void EditableMaterialProperties::init()
{
    m_materialDescs["u_shininess"] = EditableMaterialDesc(0.0f, 255.0f);
    m_materialDescs["u_emissionColor"] = EditableMaterialDesc(true);
    m_materialDescs["u_color"] = EditableMaterialDesc(true);
    m_materialDescs["u_specularColor"] = EditableMaterialDesc(true);
}

Material::Material(std::shared_ptr<Shader> shader)
    : m_shader(shader) {}

void Material::setShader(std::shared_ptr<Shader> shader)
{
    m_shader = shader;
}

void Material::setTexture2D(const TextureName& textureName, TextureID textureID)
{
    m_textures2D[textureName] = textureID;
}

void Material::setTexture3D(const TextureName& textureName, TextureID textureID)
{
    m_textures3D[textureName] = textureID;
}

void Material::use()
{
    use(m_shader.get(), true);
}

void Material::use(Shader* shader, bool bind)
{
    assert(shader);

    if (bind)
        shader->bind();

    for (auto& p : m_floatMap)
        shader->setFloat(p.first, p.second);

    for (auto& p : m_vec2Map)
        shader->setVector(p.first, p.second);

    for (auto& p : m_vec3Map)
        shader->setVector(p.first, p.second);

    for (auto& p : m_vec4Map)
        shader->setVector(p.first, p.second);

    for (auto& p : m_mat2Map)
        shader->setMatrix(p.first, p.second);

    for (auto& p : m_mat3Map)
        shader->setMatrix(p.first, p.second);

    for (auto& p : m_mat4Map)
        shader->setMatrix(p.first, p.second);

    GLint textureUnit = 0;
    for (auto& tp : m_textures2D)
        shader->bindTexture2D(tp.second, tp.first, textureUnit++);

    for (auto& tp : m_textures3D)
        shader->bindTexture3D(tp.second, tp.first, textureUnit++);
}
