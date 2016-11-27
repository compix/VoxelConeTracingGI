#pragma once
#include <SDL_stdinc.h>

using Seconds = float;

class Timer
{
public:
    Timer();

    void start();
    Seconds totalTime() const;

    /**
    * @return deltaTime
    */
    Seconds tick();
    Seconds deltaTime() const;

    uint64_t totalTimeInMilliseconds() const;
    uint64_t deltaTimeInMilliseconds() const;

    uint64_t totalTimeInMicroseconds() const;
    uint64_t deltaTimeInMicroseconds() const;

private:
    uint64_t m_deltaTimeMicroseconds{0};
    uint64_t m_prevTimeMicroseconds{0};
    uint64_t m_totalTimeMicroseconds{0};
};

class Time
{
public:
    static void update();

    static Seconds deltaTime() { return m_timer.deltaTime(); }

    static Seconds totalTime() { return m_timer.totalTime(); }

    static uint64_t getTimestampInMilliseconds();
    static uint64_t getTimestampInMicroseconds();
private:
    static Timer m_timer;
};
