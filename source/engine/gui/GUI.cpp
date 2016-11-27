#include "GUI.h"
#include "engine/rendering/voxelConeTracing/tools/ConeTool.h"

std::unordered_map<GLuint, GUITexture> GUI::textures;

void GUI::showHint(const std::string& desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(450.0f);
        ImGui::TextUnformatted(desc.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
