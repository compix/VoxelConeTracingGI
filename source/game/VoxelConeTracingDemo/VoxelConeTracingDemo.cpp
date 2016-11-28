#include "VoxelConeTracingDemo.h"
#include <engine/util/Random.h>
#include <engine/util/Timer.h>
#include <engine/rendering/util/GLUtil.h>
#include <engine/Engine.h>
#include <engine/rendering/Screen.h>
#include <engine/util/morton/morton.h>
#include <engine/util/util.h>
#include <engine/resource/ResourceManager.h>
#include <fstream>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/lights/DirectionalLight.h>
#include "engine/rendering/voxelConeTracing/VoxelizationPass.h"
#include "engine/rendering/debug/DebugRenderer.h"
#include "engine/util/ECSUtil/ECSUtil.h"
#include "engine/rendering/renderPasses/SceneGeometryPass.h"
#include "engine/rendering/renderPasses/ShadowMapPass.h"
#include "engine/rendering/voxelConeTracing/RadianceInjectionPass.h"
#include "engine/rendering/voxelConeTracing/WrapBorderPass.h"
#include "engine/rendering/voxelConeTracing/GIPass.h"
#include "engine/rendering/renderPasses/ForwardScenePass.h"
#include "engine/rendering/voxelConeTracing/settings/VoxelConeTracingSettings.h"
#include "engine/util/commands/RotationCommand.h"
#include "engine/util/commands/CommandChain.h"
#include "engine/util/QueryManager.h"
#include "engine/rendering/renderer/MeshRenderers.h"

VoxelConeTracingDemo::VoxelConeTracingDemo()
{
    Input::subscribe(this);

    Random::randomize();
}

void VoxelConeTracingDemo::initUpdate()
{
    ResourceManager::setShaderIncludePath("shaders");        

    createDemoScene();

    DebugRenderer::init();

    init3DVoxelTextures();

    m_renderPipeline = std::make_unique<RenderPipeline>(MainCamera);
    m_gui = std::make_unique<VoxelConeTracingGUI>(m_renderPipeline.get());
    m_clipmapUpdatePolicy = std::make_unique<ClipmapUpdatePolicy>(ClipmapUpdatePolicy::Type::ONE_PER_FRAME_PRIORITY, CLIP_REGION_COUNT);

    // Set render pipeline input
    m_renderPipeline->putPtr("VoxelOpacity", &m_voxelOpacity);
    m_renderPipeline->putPtr("VoxelRadiance", &m_voxelRadiance);
    m_renderPipeline->putPtr("ClipRegionBBoxes", &m_clipRegionBBoxes);
    m_renderPipeline->putPtr("ClipmapUpdatePolicy", m_clipmapUpdatePolicy.get());

    updateCameraClipRegions();

    // Add render passes to the pipeline
    m_renderPipeline->addRenderPasses(
        std::make_shared<SceneGeometryPass>(),
        std::make_shared<VoxelizationPass>(),
        std::make_shared<ShadowMapPass>(SHADOW_SETTINGS.shadowMapResolution),
        std::make_shared<RadianceInjectionPass>(),
        std::make_shared<WrapBorderPass>(),
        std::make_shared<GIPass>(),
        std::make_shared<ForwardScenePass>());

    // RenderPass initializations
    m_renderPipeline->getRenderPass<VoxelizationPass>()->init(m_clipRegionBBoxExtentL0);

    m_initializing = false;
}

void VoxelConeTracingDemo::update()
{
    m_clipmapUpdatePolicy->setType(getSelectedClipmapUpdatePolicyType());
    m_clipmapUpdatePolicy->update();

    moveCamera(Time::deltaTime());

    glFrontFace(GL_CW);

    updateCameraClipRegions();

    for (auto c : ECS::getEntitiesWithComponents<CameraComponent>())
    {
        c.getComponent<CameraComponent>()->updateViewMatrix();
    }

    bool giPipeline = RENDERING_SETTINGS.pipeline.asString() == "GI";
    bool forwardPipeline = RENDERING_SETTINGS.pipeline.asString() == "Forward";
    
    if (giPipeline)
    {
        m_renderPipeline->getRenderPass<ForwardScenePass>()->setEnabled(false);
        m_renderPipeline->getRenderPass<GIPass>()->setEnabled(true);
        m_renderPipeline->getRenderPass<VoxelizationPass>()->setEnabled(true);
        m_renderPipeline->getRenderPass<RadianceInjectionPass>()->setEnabled(true);
        m_renderPipeline->getRenderPass<SceneGeometryPass>()->setEnabled(true);
    }
    
    if (forwardPipeline)
    {
        m_renderPipeline->getRenderPass<ForwardScenePass>()->setEnabled(true);
        m_renderPipeline->getRenderPass<GIPass>()->setEnabled(false);
        m_renderPipeline->getRenderPass<VoxelizationPass>()->setEnabled(true);
        m_renderPipeline->getRenderPass<RadianceInjectionPass>()->setEnabled(false);
        m_renderPipeline->getRenderPass<SceneGeometryPass>()->setEnabled(false);
    }

    animateDirLight();

    GL::setViewport(MainCamera->getViewport());

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DebugRenderInfo info(MainCamera->view(), MainCamera->proj(), MainCamera->getPosition());
    DebugRenderer::begin(info);
    DebugRenderer::update();

    glEnable(GL_DEPTH_TEST);

    m_renderPipeline->update();

    ECS::lateUpdate();

    if (m_guiEnabled)
        m_gui->update();

    m_gui->onVoxelVisualization();

    DebugRenderer::end();
}

void VoxelConeTracingDemo::moveCamera(Seconds deltaTime) const
{
    float speed = DEMO_SETTINGS.cameraSpeed * deltaTime;

    if (Input::isKeyDown(SDL_SCANCODE_W))
        MainCamera->walk(speed);

    if (Input::isKeyDown(SDL_SCANCODE_S))
        MainCamera->walk(-speed);

    if (Input::isKeyDown(SDL_SCANCODE_A))
        MainCamera->strafe(-speed);

    if (Input::isKeyDown(SDL_SCANCODE_D))
        MainCamera->strafe(speed);

    if (Input::rightDrag().isDragging())
    {
        float dx = -Input::rightDrag().getDragDelta().x;
        float dy = Input::rightDrag().getDragDelta().y;

        MainCamera->pitch(math::toRadians(dy * 0.1f));
        MainCamera->rotateY(math::toRadians(dx * 0.1f));
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    else
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void VoxelConeTracingDemo::onKeyDown(SDL_Keycode keyCode)
{
    switch (keyCode)
    {
    case SDLK_F1:
        m_guiEnabled = !m_guiEnabled;
        break;
    case SDLK_f:
        RENDERING_SETTINGS.pipeline.curItem = 1;
        break;
    case SDLK_g:
        RENDERING_SETTINGS.pipeline.curItem = 0;
        break;
    case SDLK_F5:
        m_engine->requestScreenshot();
        break;
    default: break;
    }
}

void VoxelConeTracingDemo::init3DVoxelTextures()
{
    GLint filter = GL_LINEAR;
    GLint wrapS = GL_CLAMP_TO_BORDER;
    GLint wrapT = GL_CLAMP_TO_BORDER;
    GLint wrapR = GL_CLAMP_TO_BORDER;

    // Each anisotropic voxel needs multiple values (per face = 6). Furthermore multiple clipmap regions are required.
    // Per attribute everything is stored in one 3D texture:
    // In X Direction -> voxelFace0,voxelFace1,...,voxelFace5
    // In Y Direction -> clipmap L0,L1,...,Ln
    // So the position in [0, resolution - 1]^3 of a specific inner texture is accessed by:
    // samplePos(x, y, z, faceIndex, clipmapLevel) = (x, y, z) + (faceIndex, clipmapLevel, 0) * resolution
    // To ensure correct interpolation at borders we thus need to extend the resolution of each inner texture
    // by 2 in each dimension and copy in a dedicated render pass (WrapBorderPass) the border of the other side
    // to get GL_REPEAT as the texture wrapping mode.
    GLsizei resolutionWithBorder = VOXEL_RESOLUTION + 2;

    m_voxelOpacity.create(resolutionWithBorder * FACE_COUNT, CLIP_REGION_COUNT * resolutionWithBorder, resolutionWithBorder, 
        GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, Texture3DSettings::Custom);
    m_voxelOpacity.bind();
    m_voxelOpacity.setParameteri(GL_TEXTURE_WRAP_S, wrapS);
    m_voxelOpacity.setParameteri(GL_TEXTURE_WRAP_T, wrapT);
    m_voxelOpacity.setParameteri(GL_TEXTURE_WRAP_R, wrapR);
    m_voxelOpacity.setParameteri(GL_TEXTURE_MIN_FILTER, filter);
    m_voxelOpacity.setParameteri(GL_TEXTURE_MAG_FILTER, filter);

    m_voxelRadiance.create(resolutionWithBorder * FACE_COUNT, CLIP_REGION_COUNT * resolutionWithBorder, resolutionWithBorder, 
        GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, Texture3DSettings::Custom);
    m_voxelRadiance.bind();
    m_voxelRadiance.setParameteri(GL_TEXTURE_WRAP_S, wrapS);
    m_voxelRadiance.setParameteri(GL_TEXTURE_WRAP_T, wrapT);
    m_voxelRadiance.setParameteri(GL_TEXTURE_WRAP_R, wrapR);
    m_voxelRadiance.setParameteri(GL_TEXTURE_MIN_FILTER, filter);
    m_voxelRadiance.setParameteri(GL_TEXTURE_MAG_FILTER, filter);

    GL_ERROR_CHECK();
}

BBox VoxelConeTracingDemo::getBBox(size_t clipmapLevel) const
{
    glm::vec3 center = MainCamera->getPosition();
    float halfSize = 0.5f * m_clipRegionBBoxExtentL0 * std::exp2f(float(clipmapLevel));

    return BBox(center - halfSize, center + halfSize);
}

void VoxelConeTracingDemo::createDemoScene()
{
    m_scenePosition = glm::vec3(0.0f);

    Entity camera = ECS::createEntity("Camera");
    camera.addComponent<Transform>();
    camera.addComponent<CameraComponent>();

    auto camComponent = camera.getComponent<CameraComponent>();
    auto camTransform = camera.getComponent<Transform>();

    MainCamera = camComponent;

    camComponent->setPerspective(45.0f, float(Screen::getWidth()), float(Screen::getHeight()), 0.3f, 30.0f);
    glm::vec3 cameraPositionOffset(8.625f, 6.593f, -0.456f);
    camTransform->setPosition(m_scenePosition + cameraPositionOffset);
    camTransform->setEulerAngles(glm::vec3(math::toRadians(10.236f), math::toRadians(-66.0f), 0.0f));

    m_engine->registerCamera(camComponent);

    auto shader = ResourceManager::getShader("shaders/forwardShadingPass.vert", "shaders/forwardShadingPass.frag", { "in_pos", "in_normal", "in_tangent", "in_bitangent", "in_uv" });
    auto sceneRootEntity = ECSUtil::loadMeshEntities("meshes/sponza_obj/sponza.obj", shader, "textures/sponza_textures/", glm::vec3(0.01f), true);

    if (sceneRootEntity)
        sceneRootEntity->setPosition(glm::vec3(m_scenePosition));

    m_directionalLight = ECS::createEntity("Directional Light");
    m_directionalLight.addComponent<DirectionalLight>();
    m_directionalLight.addComponent<Transform>();
    m_directionalLight.getComponent<Transform>()->setPosition(glm::vec3(0.0f, 20.0f, 0.f) + m_scenePosition);
    m_directionalLight.getComponent<Transform>()->setEulerAngles(glm::vec3(math::toRadians(72.0f), 0.0f, 0.0f));
    m_directionalLight.getComponent<DirectionalLight>()->intensity = 1.5f;
}

void VoxelConeTracingDemo::animateDirLight()
{
    if (m_directionalLight && DEMO_SETTINGS.animateLight)
    {
        auto lightTransform = m_directionalLight.getComponent<Transform>();
        glm::vec3 lightPos = lightTransform->getPosition();
        glm::quat startRotation = glm::quat(glm::transpose(glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
        glm::quat dstRotation = glm::quat(glm::transpose(glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f))));

        static std::shared_ptr<RotationCommand> rotationCommand = std::make_shared<RotationCommand>(lightTransform, startRotation, dstRotation, 10.0f);
        static std::shared_ptr<RotationCommand> rotationCommand2 = std::make_shared<RotationCommand>(lightTransform, dstRotation, startRotation, 10.0f);
        static CommandChain commandChain({rotationCommand, rotationCommand2}, true);

        commandChain(Time::deltaTime());
    }
}

void VoxelConeTracingDemo::updateCameraClipRegions()
{
    m_clipRegionBBoxes.clear();
    for (size_t i = 0; i < CLIP_REGION_COUNT; ++i)
        m_clipRegionBBoxes.push_back(getBBox(i));
}

ClipmapUpdatePolicy::Type VoxelConeTracingDemo::getSelectedClipmapUpdatePolicyType() const
{
    if (GI_SETTINGS.updateOneClipLevelPerFrame)
        return ClipmapUpdatePolicy::Type::ONE_PER_FRAME_PRIORITY;

    return ClipmapUpdatePolicy::Type::ALL_PER_FRAME;
}
