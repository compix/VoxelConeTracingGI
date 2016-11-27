#include "QueryManager.h"
#include "Logger.h"
#include <algorithm>

QueryManager::ElapsedTimeMap QueryManager::m_gpuElapsedTime;
QueryManager::ElapsedTimeMap QueryManager::m_cpuElapsedTime;
uint32_t QueryManager::m_maxHistoryCount = 1024;
Timer QueryManager::m_timer;
uint32_t QueryManager::m_writeQueryBufferIdx = 0;
uint32_t QueryManager::m_readQueryBufferIdx = 0;
InternalElapsedTimeInfo* QueryManager::m_currentTimeInfo[2]{nullptr, nullptr};

struct InternalElapsedTimeInfo
{
    InternalElapsedTimeInfo(const InternalElapsedTimeInfo&) = delete;
    InternalElapsedTimeInfo& operator=(const InternalElapsedTimeInfo&) = delete;

    InternalElapsedTimeInfo() {}
    InternalElapsedTimeInfo(const std::string& name)
    {
        info[ElapsedTimeInfoType::PER_FRAME] = ElapsedTimeInfo(name);
        info[ElapsedTimeInfoType::AVERAGE_PER_INTERVAL] = ElapsedTimeInfo(name);
        info[ElapsedTimeInfoType::MAX_PER_INTERVAL] = ElapsedTimeInfo(name);
    }

    virtual ~InternalElapsedTimeInfo() {}

    virtual void begin(uint32_t bufferIdx)
    {
        if (expectingEnd)
            LOG_ERROR("Expecting end call but begin was called: " << info[ElapsedTimeInfoType::PER_FRAME].m_name);

        expectingEnd = true;
        ++beginEndCounter;
        secureTimingObject();
    }

    virtual void end(uint32_t)
    {
        if (!expectingEnd)
            LOG_ERROR("Expecting begin call but end was called: " << info[ElapsedTimeInfoType::PER_FRAME].m_name);

        expectingEnd = false;
    }

    void setParent(InternalElapsedTimeInfo* p)
    {
        parent = p;
        parent->children.push_back(this);
    }

    void reset()
    {
        beginEndCounter = 0;
        parent = nullptr;
        children.clear();
    }

    virtual uint64_t getElapsedTimeInMicroseconds(uint32_t bufferIdx) = 0;
    virtual void secureTimingObject() = 0;

    uint64_t getElapsedTimeOfChildrenInMicroseconds(uint32_t bufferIdx)
    {
        uint64_t sum = 0;
        for (auto c : children)
            sum += c->getElapsedTimeInMicroseconds(bufferIdx);

        return sum;
    }

    bool expectingEnd{ false };
    ElapsedTimeInfoBag info;
    ElapsedTimeInfo::Entry maxEntry;
    ElapsedTimeInfo::Entry averageEntry;
    uint32_t historyUpdateRateInMilliseconds{250};
    uint32_t historyUpdateTimerInMilliseconds{ 0 };
    uint32_t historyUpdateFrameCounter{ 0 };

    // OpenGL doesn't allow nested begin/end calls but having this functionality is very convenient
    // and can be realized with a child/parent hierarchy.
    // Note: Calls like: begin0 [code...] begin1 [code...] end0 ->ERROR are currently not supported.
    // Only begin0 [code...] begin1 [code...] end1 [code...] end0 with an arbitrary amount of begin/end calls inbetween.
    std::vector<InternalElapsedTimeInfo*> children;
    InternalElapsedTimeInfo* parent{ nullptr };

    // Number of begin/end calls during the same frame
    uint32_t beginEndCounter{ 0 };
};

struct InternalElapsedTimeInfoGPU : InternalElapsedTimeInfo
{
    InternalElapsedTimeInfoGPU(const std::string& name)
        : InternalElapsedTimeInfo(name) {}

    void secureTimingObject() override
    {
        for (int i = 0; i < MAX_QUERY_OBJECT_BUFFERS; ++i)
            if (beginEndCounter > queryObjects[i].size())
            {
                queryObjects[i].push_back(std::move(std::make_unique<GLQueryObject>()));
                // To make sure that glGetQueryObject() doesn't cause GL_INVALID_OPERATION
                queryObjects[i][beginEndCounter - 1]->begin(GL_TIME_ELAPSED);
                queryObjects[i][beginEndCounter - 1]->end();
            }
    }

    void begin(uint32_t bufferIdx) override
    {
        InternalElapsedTimeInfo::begin(bufferIdx);
        queryObjects[bufferIdx][beginEndCounter - 1]->begin(GL_TIME_ELAPSED);
    }

    void end(uint32_t bufferIdx) override
    {
        InternalElapsedTimeInfo::end(bufferIdx);
        queryObjects[bufferIdx][beginEndCounter - 1]->end();
    }

    uint64_t getElapsedTimeInMicroseconds(uint32_t bufferIdx) override
    {
        GLuint64 sum = 0;

        // Get accumulated time (OpenGL timing is in nanoseconds)
        for (uint32_t i = 0; i < beginEndCounter; ++i)
            sum += queryObjects[bufferIdx][i]->getUInt64(GL_QUERY_RESULT);

        return uint64_t(sum / GLuint64(1000)) + getElapsedTimeOfChildrenInMicroseconds(bufferIdx);
    }

    std::vector<std::unique_ptr<GLQueryObject>> queryObjects[MAX_QUERY_OBJECT_BUFFERS];
};

struct InternalElapsedTimeInfoCPU : InternalElapsedTimeInfo
{
    InternalElapsedTimeInfoCPU(const std::string& name)
        : InternalElapsedTimeInfo(name) {}

    void secureTimingObject() override
    {
        for (int i = 0; i < MAX_QUERY_OBJECT_BUFFERS; ++i)
            if (beginEndCounter > timers[i].size())
            timers[i].push_back(Timer());
    }

    void begin(uint32_t bufferIdx) override
    {
        InternalElapsedTimeInfo::begin(bufferIdx);
        timers[bufferIdx][beginEndCounter - 1].start();
    }

    void end(uint32_t bufferIdx) override
    {
        InternalElapsedTimeInfo::end(bufferIdx);
        timers[bufferIdx][beginEndCounter - 1].tick();
    }

    uint64_t getElapsedTimeInMicroseconds(uint32_t bufferIdx) override
    {
        uint64_t sum = 0;

        // Get accumulated time
        for (uint32_t i = 0; i < beginEndCounter; ++i)
            sum += timers[bufferIdx][i].deltaTimeInMicroseconds();

        return sum + getElapsedTimeOfChildrenInMicroseconds(bufferIdx);
    }

    std::vector<Timer> timers[MAX_QUERY_OBJECT_BUFFERS];
};

uint64_t ElapsedTimeInfo::getAverageInMicroseconds(uint64_t intervalInMilliseconds) const
{
    if (m_curEntry < 0 || m_elapsedTimeHistory.size() <= static_cast<size_t>(m_curEntry))
        return 0;

    uint64_t firstTimeStamp = m_elapsedTimeHistory[m_curEntry].timestampInMilliseconds;
    uint64_t average = 0;
    for (uint32_t i = 0; i < m_historySize; ++i)
    {
        int idx = (m_curEntry - i + m_historySize) % m_historySize;
        uint64_t interval = firstTimeStamp - m_elapsedTimeHistory[idx].timestampInMilliseconds;
        average += m_elapsedTimeHistory[idx].elapsedTimeInMicroseconds;
        if (interval >= intervalInMilliseconds)
            return average / (i + 1);
    }

    return average / m_historySize;
}

uint64_t ElapsedTimeInfo::getMaxInMicroseconds(uint64_t intervalInMilliseconds) const
{
    if (m_curEntry < 0 || m_elapsedTimeHistory.size() <= static_cast<size_t>(m_curEntry))
        return 0;

    uint64_t firstTimeStamp = m_elapsedTimeHistory[m_curEntry].timestampInMilliseconds;
    uint64_t highest = 0;
    for (uint32_t i = 0; i < m_historySize; ++i)
    {
        int idx = (m_curEntry - i + m_historySize) % m_historySize;
        uint64_t interval = firstTimeStamp - m_elapsedTimeHistory[idx].timestampInMilliseconds;

        if (highest < m_elapsedTimeHistory[idx].elapsedTimeInMicroseconds)
            highest = m_elapsedTimeHistory[idx].elapsedTimeInMicroseconds;

        if (interval >= intervalInMilliseconds)
            return highest;
    }

    return highest;
}

std::vector<ElapsedTimeInfo::Entry> ElapsedTimeInfo::getHistory(int start, int end) const
{
    assert(start <= end);

    if (m_curEntry < 0 || m_elapsedTimeHistory.size() <= static_cast<size_t>(m_curEntry))
        return std::vector<Entry>();

    start = std::max(start, -int(m_historySize));
    end = std::max(end, -int(m_historySize));
    uint32_t count = (end - start) + 1; // + 1 to include end

    std::vector<Entry> history;
    for (uint32_t i = 0; i < m_historySize && i < count; ++i)
    {
        int idx = (m_curEntry + 1 + i + start + m_historySize) % m_historySize;
        history.push_back(m_elapsedTimeHistory[idx]);
    }

    return history;
}

void ElapsedTimeInfo::add(Entry e, uint32_t maxHistoryCount)
{
    if (m_historySize < maxHistoryCount)
    {
        ++m_historySize;
        m_elapsedTimeHistory.resize(maxHistoryCount);
    }

    m_curEntry = (m_curEntry + 1) % m_historySize;
    m_elapsedTimeHistory[m_curEntry] = e;
    m_addedEntryCount++;
}

void QueryManager::update()
{
    m_writeQueryBufferIdx = (m_writeQueryBufferIdx + 1) % MAX_QUERY_OBJECT_BUFFERS;

    if (m_writeQueryBufferIdx == m_readQueryBufferIdx)
    {
        uint64_t curTimeInMilliseconds = Time::getTimestampInMilliseconds();

        updateEntries(m_gpuElapsedTime, curTimeInMilliseconds);
        updateEntries(m_cpuElapsedTime, curTimeInMilliseconds);

        m_readQueryBufferIdx = (m_readQueryBufferIdx + 1) % MAX_QUERY_OBJECT_BUFFERS;
    }

    for (auto& p : m_cpuElapsedTime)
        p.second->reset();

    for (auto& p : m_gpuElapsedTime)
        p.second->reset();

    for (int i = 0; i < 2; ++i)
        m_currentTimeInfo[i] = nullptr;

    m_timer.tick();
}

void QueryManager::reserveElapsedTime(QueryTarget target, const std::string& name)
{
    auto entryMap = getElapsedTimeMap(target);

    auto it = entryMap->find(name);
    if (it == entryMap->end())
    {
        (*entryMap)[name] = getNewEntry(target, name);
    }
    else
        LOG_ERROR("Attempting to reserve a reserved name: " << name);
}

void QueryManager::beginElapsedTime(QueryTarget target, const std::string& name)
{
    auto entryMap = getElapsedTimeMap(target);
    auto it = entryMap->find(name);

    if (it == entryMap->end())
    {
        reserveElapsedTime(target, name);
        it = entryMap->find(name);
    }

    auto curInfo = m_currentTimeInfo[int(target)];
    if (curInfo && curInfo != it->second.get())
        it->second->setParent(curInfo);

    if (it->second->parent)
        it->second->parent->end(m_writeQueryBufferIdx);

    m_currentTimeInfo[int(target)] = it->second.get();

    it->second->begin(m_writeQueryBufferIdx);
}

void QueryManager::endElapsedTime(QueryTarget target, const std::string& name)
{
    auto entryMap = getElapsedTimeMap(target);
    auto it = entryMap->find(name);

    if (it != entryMap->end())
    {
        it->second->end(m_writeQueryBufferIdx);
        if (it->second->parent)
            it->second->parent->begin(m_writeQueryBufferIdx);

        m_currentTimeInfo[int(target)] = it->second->parent;
    }
    else
        LOG_ERROR("Could not find query: " << name);
}

std::vector<ElapsedTimeInfoBag> QueryManager::getElapsedTimeInfo(QueryTarget target)
{
    ElapsedTimeMap* entryMap = getElapsedTimeMap(target);

    std::vector<ElapsedTimeInfoBag> info;

    for (auto& p : *entryMap)
        info.push_back(p.second->info);

    return info;
}

QueryManager::ElapsedTimeMap* QueryManager::getElapsedTimeMap(QueryTarget target)
{
    switch (target)
    {
    case QueryTarget::CPU:
        return &m_cpuElapsedTime;
    case QueryTarget::GPU:
        return &m_gpuElapsedTime;
    default:
        assert(false);
        break;
    }

    return nullptr;
}

std::unique_ptr<InternalElapsedTimeInfo> QueryManager::getNewEntry(QueryTarget target, const std::string& name)
{
    switch (target)
    {
    case QueryTarget::CPU:
        return std::move(std::make_unique<InternalElapsedTimeInfoCPU>(name));
    case QueryTarget::GPU:
        return std::move(std::make_unique<InternalElapsedTimeInfoGPU>(name));
    default: break;
    }

    return nullptr;
}

void QueryManager::updateEntries(ElapsedTimeMap& entryMap, uint64_t curTimeInMilliseconds)
{
    for (auto& p : entryMap)
    {
        auto& info = p.second;
        assert(!info->expectingEnd);

        ElapsedTimeInfo::Entry entry(info->getElapsedTimeInMicroseconds(m_readQueryBufferIdx), curTimeInMilliseconds);
        info->info[ElapsedTimeInfoType::PER_FRAME].add(entry, m_maxHistoryCount);

        if (info->maxEntry.elapsedTimeInMicroseconds < entry.elapsedTimeInMicroseconds)
            info->maxEntry = entry;

        // Compute a cumulative moving average
        info->averageEntry.elapsedTimeInMicroseconds = (entry.elapsedTimeInMicroseconds + info->historyUpdateFrameCounter * info->averageEntry.elapsedTimeInMicroseconds) / (info->historyUpdateFrameCounter + 1);

        info->historyUpdateTimerInMilliseconds += uint32_t(m_timer.deltaTimeInMilliseconds());
        info->historyUpdateFrameCounter++;

        if (info->historyUpdateTimerInMilliseconds >= info->historyUpdateRateInMilliseconds)
        {
            info->averageEntry.timestampInMilliseconds = curTimeInMilliseconds;
            info->maxEntry.timestampInMilliseconds = curTimeInMilliseconds;
            info->info[ElapsedTimeInfoType::AVERAGE_PER_INTERVAL].add(info->averageEntry, m_maxHistoryCount);
            info->info[ElapsedTimeInfoType::MAX_PER_INTERVAL].add(info->maxEntry, m_maxHistoryCount);

            info->historyUpdateTimerInMilliseconds = 0;
            info->maxEntry = ElapsedTimeInfo::Entry(0, curTimeInMilliseconds);
            info->averageEntry = ElapsedTimeInfo::Entry(0, curTimeInMilliseconds);
            info->historyUpdateFrameCounter = 0;
        }
    }
}
