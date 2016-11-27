#pragma once
#include <vector>

class ClipmapUpdatePolicy
{
public:
    enum class Type
    {
        ALL_PER_FRAME,
        ONE_PER_FRAME_PRIORITY // Update Level 0 every 2 frames, level 1 every 4 frames, level 2 every 8 frames...
    };

public:
    ClipmapUpdatePolicy(Type type, int clipRegionCount)
        : m_type(type),
          m_clipRegionCount(clipRegionCount) {}

    void update();

    void setType(Type type) { m_type = type; }
    Type getType() const { return m_type; }
    const std::vector<int>& getLevelsScheduledForUpdate() const { return m_levelsScheduledForUpdate; }

private:
    void updateAll();
    void updateOnePriority();

private:
    Type m_type{ Type::ONE_PER_FRAME_PRIORITY };
    std::vector<int> m_levelsScheduledForUpdate;
    int m_frameCounter{ 0 };
    int m_clipRegionCount;
};
