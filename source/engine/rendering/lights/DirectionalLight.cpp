#include "DirectionalLight.h"
#include <imgui/imgui.h>

void DirectionalLight::onShowInEditor()
{
    ImGui::ColorEdit3("Color", &color[0]);
    ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 20.0f);
    ImGui::DragFloat("PCF Radius", &pcfRadius, 0.0001f, 0.0f, 0.02f);
    ImGui::Checkbox("Shadows Enabled", &shadowsEnabled);

    ImGui::DragFloat("Z-Near", &zNear, 0.1f, 0.1f, 5.0f);
    ImGui::DragFloat("Z-Far", &zFar, 0.1f, 0.1f, 100.0f);
}
