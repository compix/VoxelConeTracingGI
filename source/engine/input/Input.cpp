#include "Input.h"
#include <SDL.h>
#include <algorithm>
#include <assert.h>

DragState Input::m_rightDragState(SDL_BUTTON_RIGHT);
DragState Input::m_leftDragState(SDL_BUTTON_LEFT);
std::vector<InputHandler*> Input::m_inputHandlers;
glm::vec3 Input::mousePosition;
std::vector<SDL_Event> Input::m_sdlEvents;

DragState::DragState(uint32_t sdlButton)
    : m_dragging(false), m_dragStarted(false), m_dragStopped(false), m_sdlButton(sdlButton)
{
    
}

void DragState::update(int screenHeight, bool flipMouseY)
{
    m_dragStarted = m_dragStopped = false;

    m_lastDragPos = m_curDragPos;

    int x, y;
    bool dragging = getDragInput(&x, &y);
    y = flipMouseY ? (screenHeight - y) : y;

    if (!m_dragging && dragging)
    {
        m_dragStarted = true;
        m_curDragPos = m_lastDragPos = glm::vec2(float(x), float(y));
        m_startDragPos = m_curDragPos;
    }

    if (m_dragging && !dragging) { m_dragStopped = true; }

    m_dragging = dragging;

    if (m_dragging)
        m_curDragPos = glm::vec2(float(x), float(y));
}

bool DragState::getDragInput(int* x, int* y) const
{
    return (SDL_GetMouseState(x, y) & SDL_BUTTON(m_sdlButton)) != 0;
}

void Input::update(int screenHeight, bool flipMouseY)
{
    assert(flipMouseY ? screenHeight > 0 : true);

    // Need to poll events first to provide correct mouse information (position, drag info)
    // Otherwise SDL_GetMouseState yields information from the last frame
    SDL_Event sdlEvent;
    m_sdlEvents.clear();
    while (SDL_PollEvent(&sdlEvent))
        m_sdlEvents.push_back(sdlEvent);

    int x, y;
    SDL_GetMouseState(&x, &y);
    mousePosition = glm::vec3(float(x), flipMouseY ? (screenHeight - y) : float(y), 1.f);
    m_rightDragState.update(screenHeight, flipMouseY);
    m_leftDragState.update(screenHeight, flipMouseY);

    if (m_rightDragState.dragStarted() || m_leftDragState.dragStarted())
        SDL_CaptureMouse(SDL_TRUE);

    if ((m_rightDragState.dragStopped() || m_leftDragState.dragStopped()) && !m_rightDragState.isDragging() && !m_leftDragState.isDragging())
        SDL_CaptureMouse(SDL_FALSE);

    for (auto& e : m_sdlEvents)
    {
        for (auto h : m_inputHandlers)
        {
            h->onSDLEvent(e);

            switch (e.type)
            {
            case SDL_QUIT:
                 h->onQuit();
                break;
            case SDL_MOUSEWHEEL:
                h->onMousewheel(float(e.wheel.y));
                break;
            case SDL_KEYDOWN:
                h->onKeyDown(e.key.keysym.sym);
                break;
            case SDL_WINDOWEVENT:
                h->onWindowEvent(e.window);
                break;
            case SDL_MOUSEBUTTONDOWN:
                h->onMouseDown(e.button);
                break;
            case SDL_MOUSEBUTTONUP:
                h->onMouseUp(e.button);
                break;
            case SDL_MOUSEMOTION:
                h->onMouseMotion(e.motion);
                break;
            default: break;
            }
        }
    }
}

void Input::subscribe(InputHandler* inputHandler)
{
    m_inputHandlers.push_back(inputHandler);
}

void Input::unsubscribe(InputHandler* inputHandler)
{
    m_inputHandlers.erase(std::remove(m_inputHandlers.begin(), m_inputHandlers.end(), inputHandler),
                          m_inputHandlers.end());
}

bool Input::isKeyDown(SDL_Scancode scancode) noexcept
{
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    return keyState[scancode] != 0;
}
