#pragma once
#include "Window.h"
#include <memory>

class Screen
{
public:
    static void init(int width, int height, bool enableVsync);

    static int getWidth() { return m_window->getWidth(); }

    static int getHeight() { return m_window->getHeight(); }

    static void update() { m_window->flip(); }

    static void setTitle(const std::string& title) { m_window->setTitle(title); }

    static void setVsync(bool enableVsync) { m_window->setVsync(enableVsync); }

    static void resize(int width, int height) { m_window->resize(width, height); }

    static void showMessageBox(const std::string& title, const std::string& message, Uint32 flags = SDL_MESSAGEBOX_INFORMATION)
    {
        m_window->showMessageBox(title, message, flags);
    }

    static void showErrorBox(const std::string& title, const std::string& message) { m_window->showErrorBox(title, message); }

    static SDL_Window* getSDLWindow() { return m_window->getSDLWindow(); }

private:
    static std::unique_ptr<Window> m_window;
};
