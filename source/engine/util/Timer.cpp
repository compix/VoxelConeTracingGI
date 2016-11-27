#include "Timer.h"
#include <SDL.h>

//#define USE_SDL2_TIMING
#define USE_CHRONO_TIMING

#ifdef USE_CHRONO_TIMING
#include <chrono>
using namespace std::chrono;
#endif

Timer Time::m_timer;

uint64_t getCurrentTimeInMicroseconds()
{
#ifdef USE_CHRONO_TIMING
    return static_cast<uint64_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
#elif defined(USE_SDL2_TIMING)
    return static_cast<uint64_t>(SDL_GetTicks() * 1000);
#endif
}

Timer::Timer()
    : m_prevTimeMicroseconds(getCurrentTimeInMicroseconds())
{
    
}

void Timer::start()
{
    m_prevTimeMicroseconds = getCurrentTimeInMicroseconds();
    m_deltaTimeMicroseconds = 0;
    m_totalTimeMicroseconds = 0;
}

Seconds Timer::totalTime() const
{
    return m_totalTimeMicroseconds / static_cast<Seconds>(1e6);
}

Seconds Timer::tick()
{
    auto curTime = getCurrentTimeInMicroseconds();
    m_deltaTimeMicroseconds = curTime - m_prevTimeMicroseconds;
    m_totalTimeMicroseconds += m_deltaTimeMicroseconds;

    m_prevTimeMicroseconds = curTime;
    return m_deltaTimeMicroseconds / Seconds(1e6);
}

Seconds Timer::deltaTime() const
{
    return m_deltaTimeMicroseconds / Seconds(1e6);
}

uint64_t Timer::totalTimeInMilliseconds() const
{
    return m_totalTimeMicroseconds / 1000;
}

uint64_t Timer::deltaTimeInMilliseconds() const
{
    return m_deltaTimeMicroseconds / 1000;
}

uint64_t Timer::totalTimeInMicroseconds() const
{
    return m_totalTimeMicroseconds;
}

uint64_t Timer::deltaTimeInMicroseconds() const
{
    return m_deltaTimeMicroseconds;
}

void Time::update()
{
    m_timer.tick();
}

uint64_t Time::getTimestampInMilliseconds()
{
    return getCurrentTimeInMicroseconds() / 1000;
}

uint64_t Time::getTimestampInMicroseconds()
{
    return getCurrentTimeInMicroseconds();
}
