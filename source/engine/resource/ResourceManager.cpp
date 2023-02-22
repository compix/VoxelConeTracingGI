#include "ResourceManager.h"
#include "AssetImporter.h"
#include <engine/rendering/Material.h>
#include <engine/rendering/renderer/MeshRenderer.h>
#include "engine/util/file.h"
#include <cstddef>

std::unordered_map<Texture2DKey, std::shared_ptr<Texture2D>> ResourceManager::m_textures2D;
std::unordered_map<std::string, std::shared_ptr<MeshRenderer>> ResourceManager::m_meshRenderers;
std::unordered_map<std::string, std::shared_ptr<Model>> ResourceManager::m_models;
std::unordered_map<ShaderKey, std::shared_ptr<Shader>> ResourceManager::m_shaders;
std::unordered_map<std::string, std::string> ResourceManager::m_shaderIncludes;

std::shared_ptr<Texture2D> ResourceManager::getTexture(const std::string& path, Texture2DSettings settings)
{
    auto key = Texture2DKey(path, settings);
    auto it = m_textures2D.find(key);
    if (it != m_textures2D.end())
        return it->second;

    auto texture = std::make_shared<Texture2D>(ASSET_ROOT_FOLDER + path, settings);
    m_textures2D[key] = texture;

    return texture;
}

void ResourceManager::setTextures(const std::string& textureName, const std::string& baseTexturePath,
    const std::vector<std::string>& texturePaths, Material* material, Texture2DSettings settings)
{
    for (std::size_t i = 0; i < texturePaths.size(); ++i)
    {
        auto texture = getTexture(baseTexturePath + texturePaths[i], settings);

        if (texture->isValid())
            material->setTexture2D(textureName + std::to_string(i), *texture);
    }
}

std::shared_ptr<MeshRenderer> ResourceManager::getMeshRenderer(const std::string& path, std::shared_ptr<Shader> shader, const std::string& baseTexturePath)
{
    auto it = m_meshRenderers.find(path);
    if (it != m_meshRenderers.end())
        return it->second;

    auto model = getModel(path);

    auto mesh = std::make_shared<Mesh>();

    mesh->setSubMeshes(model->getAllSubMeshes());
    mesh->finalize();

    auto meshRenderer = std::make_shared<MeshRenderer>();
    meshRenderer->setMesh(mesh);

    std::vector<MaterialDescription> materialDescs = model->getAllMaterials();

    for (std::size_t i = 0; i < materialDescs.size(); ++i)
    {
        auto material = std::make_shared<Material>(shader);
        auto& materialDesc = materialDescs[i];

        material->setFloat("u_hasDiffuseTexture", materialDesc.diffuseTextures.size() > 0);
        material->setFloat("u_hasNormalMap", materialDesc.normalTextures.size() > 0);
        material->setFloat("u_hasSpecularMap", materialDesc.specularTextures.size() > 0);
        material->setFloat("u_hasEmissionMap", materialDesc.emissionTextures.size() > 0);
        material->setFloat("u_hasOpacityMap", materialDesc.opacityTextures.size() > 0);
        material->setFloat("u_shininess", materialDesc.shininess);

        setTextures("u_diffuseTexture", baseTexturePath, materialDesc.diffuseTextures, material.get(), Texture2DSettings::S_T_REPEAT_ANISOTROPIC);
        setTextures("u_normalMap", baseTexturePath, materialDesc.normalTextures, material.get(), Texture2DSettings::S_T_REPEAT_ANISOTROPIC);
        setTextures("u_specularMap", baseTexturePath, materialDesc.specularTextures, material.get(), Texture2DSettings::S_T_REPEAT_MIN_MIPMAP_LINEAR_MAG_LINEAR);
        setTextures("u_emissionMap", baseTexturePath, materialDesc.emissionTextures, material.get(), Texture2DSettings::S_T_REPEAT_ANISOTROPIC);
        setTextures("u_opacityMap", baseTexturePath, materialDesc.opacityTextures, material.get(), Texture2DSettings::S_T_REPEAT_MIN_MAG_NEAREST);

        meshRenderer->addMaterial(material);
    }

    m_meshRenderers[path] = meshRenderer;
    return meshRenderer;
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string& path)
{
    auto it = m_models.find(path);
    if (it != m_models.end())
        return it->second;

    auto model = AssetImporter::import(ASSET_ROOT_FOLDER + path);
    m_models[path] = model;

    return model;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& vsPath, const std::string& fsPath, std::initializer_list<std::string> vertexAttributeNames)
{
    auto key = ShaderKey(vsPath, fsPath);
    auto it = m_shaders.find(key);
    if (it != m_shaders.end())
        return it->second;

    std::shared_ptr<Shader> shader = std::make_shared<Shader>();

    shader->load(ASSET_ROOT_FOLDER + vsPath, ASSET_ROOT_FOLDER + fsPath, vertexAttributeNames);
    m_shaders[key] = shader;

    return shader;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& vsPath, const std::string& fsPath, const std::string& gsPath)
{
    auto key = ShaderKey(vsPath, fsPath, gsPath);
    auto it = m_shaders.find(key);
    if (it != m_shaders.end())
        return it->second;

    std::shared_ptr<Shader> shader = std::make_shared<Shader>();

    shader->load(ASSET_ROOT_FOLDER + vsPath, ASSET_ROOT_FOLDER + fsPath, ASSET_ROOT_FOLDER + gsPath);
    m_shaders[key] = shader;

    return shader;
}

std::shared_ptr<Shader> ResourceManager::getComputeShader(const std::string& path)
{
    auto key = ShaderKey(path);
    auto it = m_shaders.find(key);
    if (it != m_shaders.end())
        return it->second;

    std::shared_ptr<Shader> shader = std::make_shared<Shader>();

    shader->loadCompute(ASSET_ROOT_FOLDER + path);
    m_shaders[key] = shader;

    return shader;
}

void ResourceManager::setShaderIncludePath(const std::string& path)
{
    const std::string fullPath = ASSET_ROOT_FOLDER + path;

    if (!file::exists(fullPath))
    {
        LOG_ERROR("The path " << path << " does not exist.");
        return;
    }

    std::size_t pathLength = fullPath.length();

    file::forEachFileInDirectory(fullPath, true, [pathLength](const std::string& directoryPath, const std::string& filename, bool isDirectory) 
    {
        if (!isDirectory)
        {
            file::Path p(filename);

            if (p.getExtension() == ".glsl")
            {
                std::string fPath = directoryPath + "/" + filename;
                std::string rPath(fPath.substr(pathLength));
                std::string str = file::readAsString(fPath);

                if (glewIsSupported("GL_ARB_shading_language_include") == GL_TRUE)
                    glNamedStringARB(GL_SHADER_INCLUDE_ARB, static_cast<GLint>(rPath.length()), rPath.c_str(), static_cast<GLint>(str.length()), str.c_str());
                else
                    m_shaderIncludes[rPath] = str;
            }
        }
    });
}

const std::string& ResourceManager::getIncludeSource(const std::string& includePath)
{
    return m_shaderIncludes[includePath];
}
