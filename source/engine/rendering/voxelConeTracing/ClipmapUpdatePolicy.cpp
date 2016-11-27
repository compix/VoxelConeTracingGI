#include "ClipmapUpdatePolicy.h"
#include <math.h>
#include <assert.h>

void ClipmapUpdatePolicy::update()
{
    m_levelsScheduledForUpdate.clear();

    switch (m_type)
    {
    case Type::ALL_PER_FRAME: 
        updateAll();
        break;
    case Type::ONE_PER_FRAME_PRIORITY: 
        updateOnePriority();
        break;
    default: 
        assert(false);
        break;
    }

    ++m_frameCounter;

    if (m_frameCounter == static_cast<int>(exp2(m_clipRegionCount - 1)))
        m_frameCounter = 0;
}

void ClipmapUpdatePolicy::updateAll()
{
    for (int i = 0; i < m_clipRegionCount; ++i)
    {
        m_levelsScheduledForUpdate.push_back(i);
    }
}

void ClipmapUpdatePolicy::updateOnePriority()
{
    // Update Level 0 every 2 frames, level 1 every 4 frames, level 2 every 8 frames...
    for (int i = 0; i < m_clipRegionCount; ++i)
    {
        int start = static_cast<int>(exp2(i)) - 1;

        int frequency = static_cast<int>(exp2(i + 1));
        if (i + 1 == m_clipRegionCount)
            frequency = static_cast<int>(exp2(i));

        if ((m_frameCounter - start) % frequency == 0)
        {
            m_levelsScheduledForUpdate.push_back(i);
            break;
        }
    }
}
