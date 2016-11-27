#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "engine/gui/GUIElements.h"
#include "engine/rendering/voxelConeTracing/Globals.h"

struct VCTSettings
{
    std::vector<GUIElement*> guiElements;
};

struct ShadowSettings : VCTSettings
{
    ShadowSettings() { guiElements.insert(guiElements.end(), {&usePoissonFilter, &depthBias, &radianceVoxelizationPCFRadius}); }

    CheckBox usePoissonFilter{"Use Poisson Filter", true};

    uint32_t shadowMapResolution{4096}; // 8184

    SliderFloat depthBias{"Depth Bias", 0.013f, 0.0001f, 0.1f, "%.6f"};
    SliderFloat radianceVoxelizationPCFRadius{ "Radiance Voxelization PCF Radius", 0.5f / VOXEL_RESOLUTION, 0.0f, 2.0f / VOXEL_RESOLUTION };
};

struct RenderingSettings : VCTSettings
{
    RenderingSettings() { guiElements.insert(guiElements.end(), {&wireFrame, &pipeline, &cullBackFaces, &brdfMode }); }

    CheckBox wireFrame{"Wireframe", false};
    ComboBox pipeline = ComboBox("Pipeline", {"GI", "Forward"}, 0);
    CheckBox cullBackFaces{ "Cull Back Faces", false };
    ComboBox brdfMode = ComboBox("BRDF", { "Blinn-Phong", "Cook-Torrance"}, 1);
};

struct VisualizationSettings : VCTSettings
{
    VisualizationSettings() { guiElements.insert(guiElements.end(), {&voxelVisualizationAlpha, &borderWidth, &borderColor}); }

    SliderFloat voxelVisualizationAlpha{"Voxel Alpha", 1.0f, 0.0f, 1.0f};
    SliderFloat borderWidth{"Border Width", 0.05f, 0.0f, 1.0f};
    ColorSelection borderColor{"Border Color", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)};
};

struct GISettings : VCTSettings
{
    GISettings()
    {
        guiElements.insert(guiElements.end(), {&occlusionDecay, &ambientOcclusionFactor, &stepFactor,
                          &indirectDiffuseIntensity, &indirectSpecularIntensity, &traceStartOffset,
                          &directLighting, &indirectDiffuseLighting, &indirectSpecularLighting, &ambientOcclusion,
                          &radianceInjectionMode, &visualizeMinLevelSelection, &downsampleTransitionRegionSize,
                          &updateOneClipLevelPerFrame });
    }

    SliderFloat occlusionDecay{"Occlusion Decay", 5.0f, 0.001f, 80.0f};
    SliderFloat ambientOcclusionFactor{ "Ambient Occlusion Factor", 2.0f, 0.1f, 4.0f };
    SliderFloat stepFactor{"Step Factor", 1.0f, 0.1f, 2.0f};
    SliderFloat indirectDiffuseIntensity{"Indirect Diffuse Intensity", 15.0f, 1.0f, 30.0f};
    SliderFloat indirectSpecularIntensity{ "Indirect Specular Intensity", 2.0f, 1.0f, 16.0f };
    SliderFloat traceStartOffset{"Trace Start Offset", 1.0f, 0.0f, 8.0f};
    CheckBox directLighting{ "Direct Lighting", true };
    CheckBox indirectDiffuseLighting{ "Indirect Diffuse Lighting", true };
    CheckBox indirectSpecularLighting{ "Indirect Specular Lighting", true };
    CheckBox ambientOcclusion{ "Ambient Occlusion", true };
    ComboBox radianceInjectionMode = ComboBox("Radiance Injection Mode", { "Conservative", "MSAA" }, 1);
    CheckBox visualizeMinLevelSelection{"Visualize Min Level Selection", false};
    SliderInt downsampleTransitionRegionSize{ "Downsample Transition Region Size", 10, 1, VOXEL_RESOLUTION / 4 };
    CheckBox updateOneClipLevelPerFrame{ "Update One Clip Level Per Frame", true };
};

struct DebugSettings : VCTSettings
{
    DebugSettings()
    {
        //guiElements.insert(guiElements.end(), {});
    }
};

struct DemoSettings : VCTSettings
{
    DemoSettings()
    {
        guiElements.insert(guiElements.end(), {&animateLight, &cameraSpeed });
    }

    CheckBox animateLight{ "Animate Light", false };
    SliderFloat cameraSpeed{ "Camera Speed", 5.0f, 1.0f, 15.0f };
};
