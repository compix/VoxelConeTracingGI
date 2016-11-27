#pragma once
#include <unordered_map>
#include "engine/gui/GUIElements.h"

enum class QueryTarget;
struct ElapsedTimeInfoBag;
class ElapsedTimeInfo;

class StatsWindow
{
    class ElapsedTimeGUIData
    {
    public:
        ElapsedTimeGUIData()
            :id(m_idCounter++) {}

        int selectedInfoType{ 0 };
        uint32_t id{ 0 };
    private:
        static uint32_t m_idCounter;
    };
public:
    StatsWindow();

    void update();
    
    bool& open() { return m_window.open; }

private:
    void plotHistogram(const ElapsedTimeInfo& timeInfo, const ElapsedTimeGUIData& guiData) const;
    void onElapsedTimeInfoItem(const ElapsedTimeInfoBag& timeInfo, QueryTarget target);

    std::unordered_map<std::string, ElapsedTimeGUIData>* getElapsedTimeGUIData(QueryTarget target);
private:
    GUIWindow m_window{ "Stats" };

    std::unordered_map<std::string, ElapsedTimeGUIData> m_elapsedTimeGUIDataCPU;
    std::unordered_map<std::string, ElapsedTimeGUIData> m_elapsedTimeGUIDataGPU;
    float m_maxDisplayedValue{ 50.0f };
};
