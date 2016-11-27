#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL.h>

class InputHandler
{
    friend class Input;
public:
    virtual ~InputHandler() { }

protected:
    virtual void onQuit() { }

    virtual void onMousewheel(float delta) { }

    virtual void onKeyDown(SDL_Keycode keyCode) { }

    virtual void onWindowEvent(const SDL_WindowEvent& windowEvent) { }

    virtual void onMouseDown(const SDL_MouseButtonEvent& e) { }

    virtual void onMouseUp(const SDL_MouseButtonEvent& e) { }

    virtual void onMouseMotion(const SDL_MouseMotionEvent& e) { }

    virtual void onSDLEvent(SDL_Event& sdlEvent) { }
};

class DragState
{
    friend class Input;
public:
    DragState(uint32_t sdlButton);
    void update(int screenHeight, bool flipMouseY);

    bool isDragging() const noexcept { return m_dragging; }

    bool dragStarted() const noexcept { return m_dragStarted; }

    bool dragStopped() const noexcept { return m_dragStopped; }

    const glm::vec2& getLastDragPos() const noexcept { return m_lastDragPos; }

    const glm::vec2& getCurDragPos() const noexcept { return m_curDragPos; }

    const glm::vec2& getStartDragPos() const noexcept { return m_startDragPos; }

    /**
    * Returns the vector from current position to the position in the last frame
    */
    glm::vec2 getDragDelta() const noexcept { return m_lastDragPos - m_curDragPos; }

    /**
    * Returns the vector from current position to the drag start position
    */
    glm::vec2 getDragDeltaToStart() const noexcept { return m_startDragPos - m_curDragPos; }

private:
    bool getDragInput(int* x, int* y) const;

private:
    bool m_dragging;
    bool m_dragStarted; // Started dragging in current frame
    bool m_dragStopped; // Stopped dragging in current frame
    glm::vec2 m_lastDragPos;
    glm::vec2 m_curDragPos;
    glm::vec2 m_startDragPos;
    uint32_t m_sdlButton;
};

class Input
{
public:
    static void update(int screenHeight = 0.f, bool flipMouseY = false);

    static const DragState& rightDrag() noexcept { return m_rightDragState; }

    static const DragState& leftDrag() noexcept { return m_leftDragState; }

    static bool isDragging() noexcept { return m_leftDragState.isDragging() || m_rightDragState.isDragging(); }

    static void subscribe(InputHandler* inputHandler);

    static void unsubscribe(InputHandler* inputHandler);

    static void clearInputHandlers() noexcept { m_inputHandlers.clear(); }

    static bool isKeyDown(SDL_Scancode scancode) noexcept;
public:
    static glm::vec3 mousePosition;
private:
    static DragState m_rightDragState;
    static DragState m_leftDragState;
    static std::vector<InputHandler*> m_inputHandlers;
    static std::vector<SDL_Event> m_sdlEvents;
};
