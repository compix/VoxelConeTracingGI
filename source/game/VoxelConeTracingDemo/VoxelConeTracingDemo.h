#pragma once
#include <engine/Game.h>
#include <engine/input/Input.h>
#include <engine/resource/Model.h>
#include <engine/rendering/Material.h>
#include <engine/rendering/Texture3D.h>
#include <engine/util/Timer.h>
#include <engine/geometry/BBox.h>
#include "gui/VoxelConeTracingGUI.h"
#include "engine/rendering/voxelConeTracing/ClipmapUpdatePolicy.h"
#include <cstddef>

class VoxelConeTracingDemo : public Game, InputHandler
{
public:
    VoxelConeTracingDemo();

    void update() override;
    void initUpdate() override;

    void moveCamera(Seconds deltaTime) const;

protected:
    void onKeyDown(SDL_Keycode keyCode) override;

private:
    void init3DVoxelTextures();

    BBox getBBox(std::size_t clipmapLevel) const;

    void createDemoScene();
    void animateDirLight();

    void updateCameraClipRegions();

    ClipmapUpdatePolicy::Type getSelectedClipmapUpdatePolicyType() const;

private:
    std::unique_ptr<RenderPipeline> m_renderPipeline;
    std::vector<BBox> m_clipRegionBBoxes;
    std::unique_ptr<ClipmapUpdatePolicy> m_clipmapUpdatePolicy;

    // ClipRegion extent at level 0 - next level covers twice as much space as the previous level
    float m_clipRegionBBoxExtentL0{16.0f};

    Texture3D m_voxelOpacity;
    Texture3D m_voxelRadiance;

    std::unique_ptr<VoxelConeTracingGUI> m_gui;
    bool m_guiEnabled{true};

    Entity m_camera;
    glm::vec3 m_scenePosition;
    Entity m_directionalLight;
};
