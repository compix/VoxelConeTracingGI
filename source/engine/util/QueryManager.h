#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "GLQueryObject.h"
#include "Timer.h"
#include <memory>
#include "Logger.h"

// Multiple buffers are used because querying can stall
#define MAX_QUERY_OBJECT_BUFFERS 3

struct InternalElapsedTimeInfoGPU;
struct InternalElapsedTimeInfoCPU;
struct InternalElapsedTimeInfo;

enum class QueryTarget
{
    CPU,
    GPU
};

enum class ElapsedTimeInfoType
{
    PER_FRAME,
    MAX_PER_INTERVAL,
    AVERAGE_PER_INTERVAL
};

class ElapsedTimeInfo
{
    friend class QueryManager;
    friend struct InternalElapsedTimeInfo;
public:
    struct Entry
    {
        Entry() {}
        Entry(uint64_t elapsedTime, uint64_t timeStampInMilliseconds)
            :elapsedTimeInMicroseconds(elapsedTime), timestampInMilliseconds(timeStampInMilliseconds) {}

        uint64_t elapsedTimeInMicroseconds{0};
        uint64_t timestampInMilliseconds{0};
    };
public:
    ElapsedTimeInfo() {}
    ElapsedTimeInfo(const std::string& name)
        :m_name(name) {}

    const std::string& getName() const { return m_name; }
    uint64_t getAverageInMicroseconds(uint64_t intervalInMilliseconds = 1000) const;
    uint64_t getMaxInMicroseconds(uint64_t intervalInMilliseconds = 1000) const;

    uint64_t getAddedEntryCount() const { return m_addedEntryCount; }

    /***
     * Returns the history in the interval [start, end].
     * If the interval exceeds history limits it is clamped to the available history.
     * An interval in [0, 99] returns the least recent 100 entries where 0 is the oldest and 99 the newest entry within this interval.
     * An interval in [-100, -1] returns the most recent 100 entries where -100 is the oldest and -1 the newest entry within this interval. 
     */
    std::vector<Entry> getHistory(int start, int end) const;
private:
    void add(Entry e, uint32_t maxHistoryCount);

private:
    std::string m_name;
    std::vector<Entry> m_elapsedTimeHistory; // In ms
    int m_curEntry{-1};
    uint32_t m_historySize{0};
    uint64_t m_addedEntryCount{ 0 };
};

struct ElapsedTimeInfoBag
{
    ElapsedTimeInfo info[3];

    ElapsedTimeInfo& operator[](ElapsedTimeInfoType type)
    {
        return info[size_t(type)];
    }

    const ElapsedTimeInfo& operator[](ElapsedTimeInfoType type) const
    {
        return info[size_t(type)];
    }
};

class QueryManager
{
    using ElapsedTimeMap = std::unordered_map<std::string, std::unique_ptr<InternalElapsedTimeInfo>>;
public:
    static void update();

    static void reserveElapsedTime(QueryTarget target, const std::string& name);

    static void beginElapsedTime(QueryTarget target, const std::string& name);
    static void endElapsedTime(QueryTarget target, const std::string& name);

    static std::vector<ElapsedTimeInfoBag> getElapsedTimeInfo(QueryTarget target);

private:
    static ElapsedTimeMap* getElapsedTimeMap(QueryTarget target);
    static std::unique_ptr<InternalElapsedTimeInfo> getNewEntry(QueryTarget target, const std::string& name);
    static void updateEntries(ElapsedTimeMap& entryMap, uint64_t curTimeInMilliseconds);

private:
    static ElapsedTimeMap m_gpuElapsedTime;
    static ElapsedTimeMap m_cpuElapsedTime;
    static uint32_t m_maxHistoryCount;
    static Timer m_timer;
    static uint32_t m_writeQueryBufferIdx;
    static uint32_t m_readQueryBufferIdx;
    static InternalElapsedTimeInfo* m_currentTimeInfo[2];
};
