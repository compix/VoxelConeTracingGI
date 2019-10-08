#include "ECSUtil.h"
#include <engine/ecs/ECS.h>
#include <engine/resource/ResourceManager.h>
#include <engine/util/math.h>
#include <engine/rendering/Material.h>
#include <engine/rendering/renderer/MeshRenderer.h>
#include <engine/util/util.h>
#include "engine/rendering/lights/DirectionalLight.h"
#include "engine/rendering/voxelConeTracing/Globals.h"
#include "engine/rendering/util/GLUtil.h"

void setTextures(const std::string& textureName, const std::string& baseTexturePath,
                 const std::vector<std::string>& texturePaths, Material* material, Texture2DSettings settings)
{
    for (size_t i = 0; i < texturePaths.size(); ++i)
    {
        auto texture = ResourceManager::getTexture(baseTexturePath + texturePaths[i], settings);

        if (texture->isValid())
            material->setTexture2D(textureName + std::to_string(i), *texture);
    }
}

ComponentPtr<Transform> ECSUtil::loadMeshEntities(Model* model, std::shared_ptr<Shader> shader, const std::string& baseTexturePath,
                                                  const glm::vec3& scale, bool useDerivedPos, ComponentPtr<Transform> parent)
{
    std::vector<MaterialDescription> materialDescs = model->getAllMaterials();

    // Create an entity
    Entity entity = ECS::createEntity(model->name);
    entity.addComponent<Transform>();
    auto transform = entity.getComponent<Transform>();
    transform->setLocalPosition(model->position);
    transform->setLocalRotation(model->rotation);
    transform->setLocalScale(model->scale);

    if (parent)
        transform->setParent(parent);

    if (model->subMeshes.size() > 0)
    {
        entity.addComponent<MeshRenderer>();

        auto mesh = std::make_shared<Mesh>();

        // Fill sub meshes to make them compatible with all shaders
        for (auto& subMesh : model->subMeshes)
        {
            subMesh.tangents.resize(subMesh.vertices.size());
            subMesh.bitangents.resize(subMesh.vertices.size());
            subMesh.uvs.resize(subMesh.vertices.size());
        }

        mesh->setSubMeshes(model->subMeshes);
        mesh->scale(scale);

        // When a whole scene is loaded objects in the scene can have an inconvenient transform - deriving the position attempts to fix this problem
        if (useDerivedPos)
        {
            auto center = mesh->computeCenter();
            mesh->translate(-center);
            transform->setLocalPosition(center);
        }

        mesh->finalize();

        transform->setBBox(util::computeBBox(*mesh.get()));

        auto meshRenderer = entity.getComponent<MeshRenderer>();
        meshRenderer->setMesh(mesh);

        for (auto& materialDesc : model->materials)
        {
            auto material = std::make_shared<Material>(shader);
            material->setFloat("u_hasDiffuseTexture", materialDesc.diffuseTextures.size() > 0);
            material->setFloat("u_hasNormalMap", materialDesc.normalTextures.size() > 0);
            material->setFloat("u_hasSpecularMap", materialDesc.specularTextures.size() > 0);
            material->setFloat("u_hasEmissionMap", materialDesc.emissionTextures.size() > 0);
            material->setFloat("u_hasOpacityMap", materialDesc.opacityTextures.size() > 0);
            material->setFloat("u_shininess", materialDesc.shininess);
            material->setColor("u_color", glm::vec4(materialDesc.diffuseColor, 1.0f));
            material->setColor("u_emissionColor", glm::vec3(0.0f));
            material->setColor("u_specularColor", materialDesc.specularColor);

            setTextures("u_diffuseTexture", baseTexturePath, materialDesc.diffuseTextures, material.get(), Texture2DSettings::S_T_REPEAT_ANISOTROPIC);
            setTextures("u_normalMap", baseTexturePath, materialDesc.normalTextures, material.get(), Texture2DSettings::S_T_REPEAT_ANISOTROPIC);
            setTextures("u_specularMap", baseTexturePath, materialDesc.specularTextures, material.get(), Texture2DSettings::S_T_REPEAT_MIN_MIPMAP_LINEAR_MAG_LINEAR);
            setTextures("u_emissionMap", baseTexturePath, materialDesc.emissionTextures, material.get(), Texture2DSettings::S_T_REPEAT_ANISOTROPIC);
            setTextures("u_opacityMap", baseTexturePath, materialDesc.opacityTextures, material.get(), Texture2DSettings::S_T_REPEAT_MIN_MAG_NEAREST);

            meshRenderer->addMaterial(material);
        }
    }

    for (auto& child : model->children)
    {
        loadMeshEntities(child.get(), shader, baseTexturePath, scale, useDerivedPos, transform);
    }

    return transform;
}

ComponentPtr<Transform> ECSUtil::loadMeshEntities(const std::string& path, std::shared_ptr<Shader> shader, 
    const std::string& baseTexturePath, const glm::vec3& scale, bool useDerivedPos)
{
    auto model = ResourceManager::getModel(path);
    if (!model)
        return ComponentPtr<Transform>();

    return loadMeshEntities(model.get(), shader, baseTexturePath, scale, useDerivedPos);
}

void ECSUtil::renderEntities(Shader* shader)
{
    for (auto e : ECS::getEntitiesWithComponents<Transform, MeshRenderer>())
        renderEntity(e, shader);
}

void ECSUtil::renderEntities(const std::vector<Entity>& entities, Shader* shader)
{
    for (auto& e : entities)
        renderEntity(e, shader);
}

void ECSUtil::renderEntity(Entity entity, Shader* shader)
{
    auto transform = entity.getComponent<Transform>();
    auto renderer = entity.getComponent<MeshRenderer>();
    assert(transform && renderer);

    shader->setMatrix("u_model", transform->getLocalToWorldMatrix());
    shader->setMatrix("u_modelIT", glm::transpose(glm::inverse(transform->getLocalToWorldMatrix())));

    shader->setUnsignedInt("u_entityID", entity.getID());
    shader->setUnsignedInt("u_entityVersion", entity.getVersion());

    renderer->render(shader);
}

void ECSUtil::renderEntitiesInAABB(const BBox& bbox, Shader* shader)
{
    for (Entity e : ECS::getEntitiesWithComponents<Transform, MeshRenderer>())
    {
        auto transform = e.getComponent<Transform>();

        if (bbox.overlaps(transform->getBBox()))
            renderEntity(e, shader);
    }
}

void ECSUtil::setDirectionalLightUniforms(Shader* shader, GLint shadowMapStartTextureUnit, float pcfRadius)
{
    assert(GL::isShaderBound(shader->getProgram()));

    int lightCount = 0;
    for (auto dirLight : ECS::getEntitiesWithComponents<DirectionalLight, Transform>())
    {
        if (lightCount == MAX_DIR_LIGHT_COUNT)
            break;

        auto dirLightComponent = dirLight.getComponent<DirectionalLight>();
        auto lightTransform = dirLight.getComponent<Transform>();
        std::string arrayIdxStr = "[" + std::to_string(lightCount) + "]";
        std::string dirLightUniformName = "u_directionalLights" + arrayIdxStr;
        std::string shadowUniformName = "u_directionalLightShadowDescs" + arrayIdxStr;

        shader->setVector(dirLightUniformName + ".direction", lightTransform->getForward());
        shader->setVector(dirLightUniformName + ".color", dirLightComponent->color);
        shader->setFloat(dirLightUniformName + ".intensity", dirLightComponent->intensity);

        shader->setInt(shadowUniformName + ".enabled", dirLightComponent->shadowsEnabled ? 1 : 0);

        if (dirLightComponent->shadowsEnabled)
        {
            shader->setMatrix(shadowUniformName + ".view", dirLightComponent->view);
            shader->setMatrix(shadowUniformName + ".proj", dirLightComponent->proj);
            shader->setFloat(shadowUniformName + ".zNear", dirLightComponent->zNear);
            shader->setFloat(shadowUniformName + ".zFar", dirLightComponent->zFar);
            if (pcfRadius < 0.0f)
                pcfRadius = dirLightComponent->pcfRadius;

            shader->setFloat(shadowUniformName + ".pcfRadius", pcfRadius);
            shader->bindTexture2D(dirLightComponent->shadowMap, "u_shadowMaps" + arrayIdxStr, shadowMapStartTextureUnit + lightCount);
        }

        ++lightCount;
    }

    shader->setInt("u_numActiveDirLights", lightCount);
}
