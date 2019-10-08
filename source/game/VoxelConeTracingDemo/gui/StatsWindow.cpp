#include "StatsWindow.h"
#include "engine/util/QueryManager.h"

uint32_t StatsWindow::ElapsedTimeGUIData::m_idCounter = 0;

StatsWindow::StatsWindow()
{
    m_window.open = false;
    m_window.minSize = ImVec2(250, 250);
}

void StatsWindow::update()
{
    m_window.begin();

    auto elapsedTimeCPUInfo = QueryManager::getElapsedTimeInfo(QueryTarget::CPU);
    auto elapsedTimeGPUInfo = QueryManager::getElapsedTimeInfo(QueryTarget::GPU);

    ImGui::Text("Max Displayed Value:"); ImGui::SameLine();
    ImGui::SliderFloat("", &m_maxDisplayedValue, 5.0f, 100.0f);
    ImGui::Text("CPU Elapsed Time:");
    for (auto& cpuInfo : elapsedTimeCPUInfo)
    {
        onElapsedTimeInfoItem(cpuInfo, QueryTarget::CPU);
    }

    ImGui::NewLine();
    ImGui::Text("GPU Elapsed Time:");
    for (auto& gpuInfo : elapsedTimeGPUInfo)
    {
        onElapsedTimeInfoItem(gpuInfo, QueryTarget::GPU);
    }

    m_window.end();
}

void StatsWindow::plotHistogram(const ElapsedTimeInfo& timeInfo, const ElapsedTimeGUIData& guiData) const
{
    int valueCount = 100;
    int halfValueCount = valueCount / 2;

    std::vector<float> historyF(valueCount, 0.0f);
    auto history = timeInfo.getHistory(-halfValueCount - timeInfo.getAddedEntryCount() % halfValueCount - 1, -1);
    for (size_t i = 0; i < history.size(); ++i)
        historyF[i] = history[i].elapsedTimeInMicroseconds / 1000.0f;

    if (historyF.size() > 0)
        ImGui::PlotHistogram("", &historyF[0], static_cast<int>(historyF.size()), 0, "", 0.0f, m_maxDisplayedValue, ImVec2(m_window.size.x - 50, 100.0f));
}

void StatsWindow::onElapsedTimeInfoItem(const ElapsedTimeInfoBag& timeInfo, QueryTarget target)
{
    auto name = timeInfo[ElapsedTimeInfoType::PER_FRAME].getName();

    auto guiDataMap = getElapsedTimeGUIData(target);
    auto& guiData = (*guiDataMap)[name];

    const char* infoTypes[2] = {
        "Max Per Interval",
        "Average Per Interval"
    };

    ImGui::PushID(guiData.id);
    if (ImGui::TreeNode(name.c_str()))
    {
        ElapsedTimeInfoType selectedType = ElapsedTimeInfoType::MAX_PER_INTERVAL;

        ImGui::Combo("Type", &guiData.selectedInfoType, infoTypes, 2);

        switch(guiData.selectedInfoType)
        {
        case 0:
            selectedType = ElapsedTimeInfoType::MAX_PER_INTERVAL;
            break;
        case 1:
            selectedType = ElapsedTimeInfoType::AVERAGE_PER_INTERVAL;
            break;
        default: break;
        }

        std::stringstream ss;
        ss << "Average in the last second: " << timeInfo[selectedType].getAverageInMicroseconds() / 1000.0 << " ms";
        ImGui::TextUnformatted(ss.str().c_str());
        ss.str("");
        ss.clear();
        ss << "Highest in the last second: " << timeInfo[selectedType].getMaxInMicroseconds() / 1000.0 << " ms";
        ImGui::TextUnformatted(ss.str().c_str());

        plotHistogram(timeInfo[selectedType], guiData);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

std::unordered_map<std::string, StatsWindow::ElapsedTimeGUIData>* StatsWindow::getElapsedTimeGUIData(QueryTarget target)
{
    switch(target)
    {
    case QueryTarget::CPU: 
        return &m_elapsedTimeGUIDataCPU;
    case QueryTarget::GPU: 
        return &m_elapsedTimeGUIDataGPU;
    default: 
        assert(false);
        break;
    }

    return nullptr;
}
