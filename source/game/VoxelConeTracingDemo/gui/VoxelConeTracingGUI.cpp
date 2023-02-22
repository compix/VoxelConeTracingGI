#include "VoxelConeTracingGUI.h"
#include <imgui/imgui.h>
#include <engine/rendering/Screen.h>
#include <engine/util/Timer.h>
#include <algorithm>
#include <sstream>
#include <engine/ecs/ECS.h>
#include <engine/geometry/Transform.h>
#include <engine/rendering/debug/DebugRenderer.h>
#include <engine/rendering/Texture3D.h>
#include <engine/input/Input.h>
#include "engine/util/Random.h"
#include "engine/rendering/voxelConeTracing/settings/VoxelConeTracingSettings.h"
#include "engine/rendering/voxelConeTracing/VoxelRegion.h"
#include "engine/gui/GUI.h"
#include "engine/util/ECSUtil/EntityCreator.h"
#include "engine/rendering/lights/DirectionalLight.h"
#include <cstddef>

VoxelConeTracingGUI::VoxelConeTracingGUI(RenderPipeline* renderPipeline)
    : m_renderPipeline(renderPipeline)
{
    m_mainWindow.flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    m_mainWindow.pos = ImVec2(0.0f, 20.0f);
    m_mainWindow.maxSize = ImVec2(FLT_MAX, Screen::getHeight() - 20.0f);

    m_gBuffersWindow.open = false;
    m_gBuffersWindow.minSize = ImVec2(250, 250);

    m_fpsWindow.flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    m_entityWindow.flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;
    m_entityWindow.minSize = ImVec2(300.0f, 0.0f);

    m_visualizer = std::make_unique<Visualizer>();
    m_coneTool = std::make_unique<ConeTool>();

    m_entityPicker = std::make_unique<EntityPicker>();
    m_statsWindow = std::make_unique<StatsWindow>();

    Input::subscribe(this);
}

void VoxelConeTracingGUI::update()
{
    if (m_entityPickRequest.requested)
    {
        m_entityPicker->update();
        m_selectedEntity = m_entityPicker->pick(m_entityPickRequest.x, m_entityPickRequest.y);
        m_entityPickRequest.requested = false;
    }
    
    guiShowFPS();

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImColor(0.0f, 0.0f, 0.0f, 0.8f));
    if (ImGui::BeginMainMenuBar())
    {
        showViewMenu();
        showSceneMenu();
        showEditorMenu();

        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleColor(1);

    if (m_showSettings)
    {
        m_mainWindow.begin();

        showEntityTree();

        if (m_selectedEntity)
        {
            showEntityWindow(m_selectedEntity);

            // Entity will be unselected if window is closed so check again if entity is selected!
            if (m_selectedEntity)
            {
                auto transform = m_selectedEntity.getComponent<Transform>();
                if (transform)
                {
                    DebugRenderInfo info(MainCamera->view(), MainCamera->proj(), MainCamera->getPosition());
                    DebugRenderer::begin(info);
                    if (m_showObjectCoordinateSystem)
                        DebugRenderer::drawCoordinateSystem(transform);

                    if (m_showObjectBBox)
                        DebugRenderer::drawNonFilledCube(transform->getBBox().center(), transform->getBBox().scale(), glm::vec3(0.0f, 1.0f, 0.0f));

                    DebugRenderer::end();
                }
            }
        }

        m_cameraMoveCommand(Time::deltaTime());

        showSettings("Shadow Settings", &SHADOW_SETTINGS);
        showSettings("GI Settings", &GI_SETTINGS);
        showSettings("Rendering Settings", &RENDERING_SETTINGS);
        showVisualizationSettings();
        showSettings("Debug Settings", &DEBUG_SETTINGS);
        showSettings("Demo Settings", &DEMO_SETTINGS);

        m_mainWindow.end();
    }

    if (m_statsWindow->open())
        m_statsWindow->update();

    if (m_gBuffersWindow.open)
    {
        m_gBuffersWindow.begin();

        GLuint diffuseTexture = m_renderPipeline->fetch<GLuint>("DiffuseTexture");
        GLuint normalMap = m_renderPipeline->fetch<GLuint>("NormalMap");
        GLuint specularMap = m_renderPipeline->fetch<GLuint>("SpecularMap");
        GLuint emissionMap = m_renderPipeline->fetch<GLuint>("EmissionMap");
        GLuint depthTexture = m_renderPipeline->fetch<GLuint>("DepthTexture");
        
        GUI::textures[specularMap] = GUITexture("Specular Map", specularMap, GL_RED, GL_GREEN, GL_BLUE, GL_ONE);
        GUI::textures[depthTexture] = GUITexture("Depth Texture", depthTexture, GL_RED, GL_RED, GL_RED, GL_ONE);

        showTextures(m_gBuffersWindow.size, {
                         GUITexture("Diffuse Texture", diffuseTexture),
                         GUITexture("Normal Map", normalMap),
                         GUITexture("Specular Map", specularMap),
                         GUITexture("Emission Map", emissionMap),
                         GUITexture("Depth Texture", depthTexture)});

        m_gBuffersWindow.end();
    }

    if (m_coneTool->isOpen())
    {
        m_coneTool->update();
    }
}

void VoxelConeTracingGUI::showEntityTree()
{
    m_treeNodeIdx = 0;

    if (ImGui::TreeNode("Entities"))
    {
        auto entities = ECS::getEntitiesWithComponentsIncludeInactive<Transform>();
        std::vector<ComponentPtr<Transform>> rootSet;

        for (auto entity : entities)
        {
            auto transform = entity.getComponent<Transform>();
            auto root = transform->getRoot();
            bool contains = false;

            // Make sure it's not yet in the set
            for (auto& t : rootSet)
                if (t == root)
                {
                    contains = true;
                    break;
                }

            if (!contains)
                rootSet.push_back(root);
        }

        for (auto& transform : rootSet) { subTree(transform); }

        ImGui::TreePop();
    }
}

void VoxelConeTracingGUI::moveCameraToEntity(ComponentPtr<CameraComponent>& camera, Entity& entity)
{
    auto camTransform = camera->getComponent<Transform>();
    auto entityTransform = entity.getComponent<Transform>();

    if (camTransform && entityTransform)
        m_cameraMoveCommand = MoveCommand(camTransform, camTransform->getPosition(), entityTransform->getPosition() - camTransform->getForward() * 3.0f, 0.5f);
}

void VoxelConeTracingGUI::onVoxelVisualization()
{
    bool hasMultipleFaces = true;
    int numColorComponents = 4;

    auto clipRegions = m_renderPipeline->fetchPtr<std::vector<VoxelRegion>>("ClipRegions");

    VoxelRegion prevRegion;
    bool hasPrevLevel = false;

    for (int i = 0; i < 6; ++i)
    {
        if (m_visualizeClipRegion[i])
        {
            if (m_visualizeTexture)
                m_visualizer->visualize3DTexture(*m_visualizedVoxelTex, glm::vec3(0.0f), uint32_t(i), m_padding, m_voxelSize);
            else
                m_visualizer->visualize3DClipmapGS(*m_visualizedVoxelTex, clipRegions->at(std::size_t(i)), uint32_t(i), prevRegion, hasPrevLevel, hasMultipleFaces, numColorComponents);
            hasPrevLevel = true;
            prevRegion = clipRegions->at(std::size_t(i));
        }
    }
}

void VoxelConeTracingGUI::subTree(const ComponentPtr<Transform>& transform)
{
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

    Entity owner = transform->getOwner();
    std::string entityName = owner.getName();

    ImGuiTreeNodeFlags selectedFlag = m_selectedEntity == owner ? ImGuiTreeNodeFlags_Selected : 0;

    if (transform->hasChildren())
    {
        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)m_treeNodeIdx, nodeFlags | selectedFlag, entityName.c_str(), "");
        ++m_treeNodeIdx;
        if (ImGui::IsItemClicked())
            onEntityClicked(owner);

        if (nodeOpen)
        {
            for (auto& child : transform->getChildren())
                subTree(child);

            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::TreeNodeEx((void*)(intptr_t)m_treeNodeIdx, nodeFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | selectedFlag, entityName.c_str(), "");
        if (ImGui::IsItemClicked())
            onEntityClicked(owner);

        ++m_treeNodeIdx;
    }
}

void VoxelConeTracingGUI::guiShowFPS()
{
    m_fpsWindow.begin();
    m_fpsWindow.pos.x = Screen::getWidth() * 0.5f - m_fpsWindow.size.x * 0.5f;
    m_fpsWindow.pos.y = 20.0f;

    static Seconds left = 1.f;
    static uint32_t fps = 0;
    static uint32_t fpsCounter = 0;
    static Seconds frameTime = 0.f;
    left -= Time::deltaTime();

    if (left <= 0.f)
    {
        frameTime = Seconds(1000) / std::max(fpsCounter, 1u);
        left += 1.f;
        fps = fpsCounter;
        fpsCounter = 0;
    }

    std::stringstream ss;
    ss.precision(2);

    ss << "FPS: " << fps;
    ss << " Frame time: " << std::fixed << frameTime << "ms";
    ss << " Total time: " << int(Time::totalTime());

    ImGui::TextUnformatted(ss.str().c_str());

    fpsCounter++;
    m_fpsWindow.end();
}

void VoxelConeTracingGUI::showComponents(const Entity& entity) const
{
    auto components = entity.getAllComponents();

    int treeNodeIdx = 0;

    for (auto& component : components)
    {
        if (ImGui::TreeNode((void*)(intptr_t)treeNodeIdx, component->getName().c_str(), ""))
        {
            component->onShowInEditor();
            ImGui::TreePop();
        }

        ++treeNodeIdx;
    }
}

void VoxelConeTracingGUI::showEntityWindow(const Entity& entity)
{
    m_entityWindow.open = true;

    m_entityWindow.begin();
    m_entityWindow.pos.x = Screen::getWidth() - m_entityWindow.size.x;
    m_entityWindow.pos.y = 20.0f;
    m_entityWindow.label = entity.getName();

    bool active = entity.isActive();
    ImGui::Checkbox("Active", &active);
    entity.setActive(active);

    showComponents(entity);

    if (!m_entityWindow.open)
        m_selectedEntity = Entity();

    m_entityWindow.end();
}

void VoxelConeTracingGUI::onEntityClicked(Entity& clickedEntity)
{
    m_selectedEntity = clickedEntity;

    if (ImGui::IsMouseDoubleClicked(0))
    {
        if (MainCamera)
            moveCameraToEntity(MainCamera, m_selectedEntity);
    }
}

void VoxelConeTracingGUI::showSettings(const std::string& label, const VCTSettings* settings) const
{
    if (ImGui::TreeNode(label.c_str()))
    {
        for (auto element : settings->guiElements)
        {
            element->begin();
            element->end();
        }

        ImGui::TreePop();
    }
}

void VoxelConeTracingGUI::showTextures(const ImVec2& canvasSize, std::initializer_list<GUITexture> textures) const
{
    std::size_t count = textures.size();
    std::size_t nColumns = std::size_t(ceil(sqrtf(float(count))));
    std::size_t nRows = std::size_t(ceil(float(count) / nColumns));

    auto panelSize = ImVec2(canvasSize.x / nColumns, canvasSize.y / nRows - 25.0f);
    auto textureSize = ImVec2(canvasSize.x / nColumns, canvasSize.y / nRows - 50.0f);

    std::size_t i = 0;

    for (auto& tex : textures)
    {
        if (i % nColumns > 0)
            ImGui::SameLine();

        ImGui::BeginChild(tex.label.c_str(), panelSize);
        ImGui::TextUnformatted(tex.label.c_str());
        ImGui::Image(ImTextureID(uintptr_t(tex.texID)), textureSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        ImGui::EndChild();

        ++i;
    }
}

void VoxelConeTracingGUI::showVisualizationSettings()
{
    if (ImGui::TreeNode("Visualization Settings"))
    {
        for (auto element : VISUALIZATION_SETTINGS.guiElements)
        {
            element->begin();
            element->end();
        }

        if (ImGui::TreeNode("Voxel Visualization"))
        {
            showVoxelVisualizationOptions();
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

void VoxelConeTracingGUI::showVoxelVisualizationOptions()
{
    ImGui::Checkbox("Visualize Texture", &m_visualizeTexture);
    ImGui::SliderFloat("Padding", &m_padding, 0.0f, 1.0f);
    ImGui::SliderFloat("Texel Size", &m_voxelSize, 0.0f, 1.0f);

    ImGui::Text("Visualize Clip Region:");
    ImGui::Checkbox("C1", &m_visualizeClipRegion[0]);
    ImGui::SameLine();
    ImGui::Checkbox("C2", &m_visualizeClipRegion[1]);
    ImGui::SameLine();
    ImGui::Checkbox("C3", &m_visualizeClipRegion[2]);
    ImGui::SameLine();
    ImGui::Checkbox("C4", &m_visualizeClipRegion[3]);
    ImGui::SameLine();
    ImGui::Checkbox("C5", &m_visualizeClipRegion[4]);
    ImGui::SameLine();
    ImGui::Checkbox("C6", &m_visualizeClipRegion[5]);

    static int curSelection = 0;
    static const char* voxelTextures[]
    {
        "Voxel Opacity",
        "Voxel Radiance"
    };

    ImGui::Combo("3D Texture", &curSelection, voxelTextures, 2);

    switch (curSelection)
    {
    case 0:
        m_visualizedVoxelTex = m_renderPipeline->fetchPtr<Texture3D>("VoxelOpacity");
        break;
    case 1:
        m_visualizedVoxelTex = m_renderPipeline->fetchPtr<Texture3D>("VoxelRadiance");
        break;
    default:
        assert(false);
    }
}

void VoxelConeTracingGUI::showViewMenu()
{
    if (ImGui::BeginMenu("View"))
    {
        bool coneToolOpen = m_coneTool->isOpen();

        if (ImGui::MenuItem("G-Buffers", nullptr, &m_gBuffersWindow.open))
            m_gBuffersWindow.pos = ImVec2(Screen::getWidth() * 0.5f - m_gBuffersWindow.size.x * 0.5f, Screen::getHeight() * 0.5f - m_gBuffersWindow.size.y * 0.5f);

        ImGui::MenuItem("Cone Tool", nullptr, &coneToolOpen);
        ImGui::MenuItem("Stats", nullptr, &m_statsWindow->open());
        ImGui::MenuItem("Settings", nullptr, &m_showSettings);
        ImGui::EndMenu();

        m_coneTool->setOpen(coneToolOpen);
    }
}

void VoxelConeTracingGUI::showSceneMenu()
{
    if (ImGui::BeginMenu("Scene"))
    {
        if (ImGui::BeginMenu("Add"))
        {
            glm::vec3 spawnPos = MainCamera->getPosition() + MainCamera->getForward() * 1.5f;

            if (ImGui::MenuItem("Directional Light"))
            {
                if (ECS::getEntityCountWithComponentsIncludeInactive<DirectionalLight>() == MAX_DIR_LIGHT_COUNT)
                {
                    auto msg = "Sorry, reached the maximum number of supported directional lights: " + std::to_string(MAX_DIR_LIGHT_COUNT);
                    Screen::showMessageBox("Limit reached", msg);
                }
                else
                {
                    m_selectedEntity = EntityCreator::createDefaultDirLight();
                }
            }

            if (ImGui::MenuItem("Box"))
            {
                m_selectedEntity = EntityCreator::createDefaultBox(spawnPos);
            }

            if (ImGui::MenuItem("Cylinder"))
            {
                m_selectedEntity = EntityCreator::createDefaultCylinder(spawnPos);
            }

            if (ImGui::MenuItem("Sphere"))
            {
                m_selectedEntity = EntityCreator::createDefaultSphere(spawnPos);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

void VoxelConeTracingGUI::showEditorMenu()
{
    if (ImGui::BeginMenu("Editor"))
    {
        ImGui::MenuItem("Show Object Coordinate System", nullptr, &m_showObjectCoordinateSystem);
        ImGui::MenuItem("Show Object AABB", nullptr, &m_showObjectBBox);

        ImGui::EndMenu();
    }
}

void VoxelConeTracingGUI::onMouseDown(const SDL_MouseButtonEvent& e)
{
    switch (e.button)
    {
    case SDL_BUTTON_LEFT:
        if (!ImGui::IsMouseHoveringAnyWindow())
            m_entityPickRequest = EntityPickRequest(e.x, Screen::getHeight() - e.y);
        break;
    default: break;
    }
}

void VoxelConeTracingGUI::onWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
        m_mainWindow.maxSize = ImVec2(FLT_MAX, windowEvent.data2 - 20.0f);
        break;
    default: break;
    }
}
