#pragma once
#include <memory>
#include <engine/ecs/EntityManager.h>
#include <engine/camera/CameraComponent.h>
#include <engine/geometry/Transform.h>
#include <GL/glew.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/input/Input.h>
#include "StatsWindow.h"
#include "engine/util/commands/MoveCommand.h"
#include "engine/rendering/voxelConeTracing/visualization/Visualizer.h"
#include "engine/gui/GUIElements.h"
#include "engine/rendering/voxelConeTracing/tools/ConeTool.h"
#include "engine/util/ECSUtil/EntityPicker.h"
#include "engine/gui/GUITexture.h"
#include "engine/rendering/Texture3D.h"

struct ImVec2;
struct VCTSettings;
class ElapsedTimeInfo;

class VoxelConeTracingGUI : public InputHandler
{
    struct EntityPickRequest
    {
        EntityPickRequest() {}
        EntityPickRequest(int x, int y)
            :x(x), y(y) {}

        int x{ 0 };
        int y{ 0 };

        bool requested{ true };
    };
public:
    VoxelConeTracingGUI(RenderPipeline* renderPipeline);

    void update();

    void showEntityTree();

    void moveCameraToEntity(ComponentPtr<CameraComponent>& camera, Entity& entity);

    void onVoxelVisualization();

    const std::vector<glm::vec3>& getConeDirections() const { return m_coneTool->getConeDirections(); }

private:
    void subTree(const ComponentPtr<Transform>& transform);
    void guiShowFPS();
    void showComponents(const Entity& entity) const;
    void showEntityWindow(const Entity& entity);
    void onEntityClicked(Entity& clickedEntity);
    void showSettings(const std::string& label, const VCTSettings* settings) const;
    void showTextures(const ImVec2& canvasSize, std::initializer_list<GUITexture> textures) const;
    void showVisualizationSettings();
    void showVoxelVisualizationOptions();
    void showViewMenu();
    void showSceneMenu();
    void showEditorMenu();

protected:
    void onMouseDown(const SDL_MouseButtonEvent& e) override;
    void onWindowEvent(const SDL_WindowEvent& windowEvent) override;
private:
    Entity m_selectedEntity;
    int m_treeNodeIdx{0};
    MoveCommand m_cameraMoveCommand;
    RenderPipeline* m_renderPipeline;
    std::unique_ptr<Visualizer> m_visualizer;

    bool m_showGBuffers{false};
    GUIWindow m_mainWindow{"Main Window"};
    GUIWindow m_gBuffersWindow{"G-Buffers"};
    GUIWindow m_fpsWindow{"FPS"};
    GUIWindow m_entityWindow{"Entity Window"};
    GUIWindow m_hemisphereConeToolWindow{"Cone Tool"};

    std::unique_ptr<ConeTool> m_coneTool;
    std::unique_ptr<EntityPicker> m_entityPicker;
    std::unique_ptr<StatsWindow> m_statsWindow;
    bool m_showSettings{ true };

    EntityPickRequest m_entityPickRequest;

    bool m_visualizeTexture{ false };
    float m_padding{ 0.125f };
    float m_voxelSize{ 0.125f };
    Texture3D* m_visualizedVoxelTex{ nullptr };
    bool m_visualizeClipRegion[6]{ false, false, false, false, false, false };

    bool m_showObjectCoordinateSystem{ true };
    bool m_showObjectBBox{ true };
};
