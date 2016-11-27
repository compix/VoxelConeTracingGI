#include "Screen.h"

std::unique_ptr<Window> Screen::m_window;

void Screen::init(int width, int height, bool enableVsync)
{
    m_window = std::make_unique<Window>(width, height, enableVsync);
}
