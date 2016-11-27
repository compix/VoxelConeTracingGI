#pragma once
#include <SDL.h>

class Engine;

class Game
{
    friend class Engine;
public:
    virtual ~Game() { }

    Game();

    virtual void handleSdlEvent(const SDL_Event&) { }

protected:
    virtual void update() = 0;
    virtual void initUpdate() = 0;
    virtual void quit() {}

    bool isInitializing() const { return m_initializing; }

protected:
    bool m_initializing;
    Engine* m_engine{nullptr};
};
