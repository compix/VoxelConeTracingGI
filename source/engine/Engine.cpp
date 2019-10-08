#include "Engine.h"
#include "Game.h"
#include "util/Logger.h"
#include "input/Input.h"
#include "camera/CameraComponent.h"
#include "rendering/Screen.h"
#include "ecs/ECS.h"
#include "imgui_impl/imgui_impl_sdl_gl3.h"
#include <imgui/imgui.h>
#include "rendering/util/Mipmapper.h"
#include "rendering/renderer/MeshRenderers.h"
#include "util/QueryManager.h"
#include "util/file.h"
#include "rendering/voxelConeTracing/VoxelConeTracing.h"
#include "util/colors.h"
#include <SOIL2.h>

Engine::Engine()
    : m_running(true), m_initialized(false) 
{
}

void Engine::init(Game* game)
{
    assert(game);
    m_game = game;
    m_game->m_engine = this;
    Screen::init(1100, 600, false);
    m_initialized = true;
    Input::subscribe(this);
    MeshRenderers::init();
    Mipmapper::init();
    VoxelConeTracing::init();

    ImGui_ImplSdlGL3_Init(Screen::getSDLWindow());

    EditableMaterialProperties::init();
}

void Engine::update()
{
    if (!m_initialized)
    {
        LOG_EXIT("Engine is not initialized. Shutting down...");
    }

    if (!m_paused)
    {
        QueryManager::beginElapsedTime(QueryTarget::CPU, "Total Time");
        QueryManager::beginElapsedTime(QueryTarget::GPU, "Total Time");
    }

    Time::update();
    Input::update(Screen::getHeight(), true);

    if (m_paused)
        return;

    if (m_game->isInitializing())
    {
        m_game->initUpdate();
    }
    else
    {
        ImGui_ImplSdlGL3_NewFrame(Screen::getSDLWindow());

        QueryManager::beginElapsedTime(QueryTarget::CPU, "ECS Update");
        ECS::update();
        QueryManager::endElapsedTime(QueryTarget::CPU, "ECS Update");

        m_game->update();

        QueryManager::beginElapsedTime(QueryTarget::CPU, "GUI");
        QueryManager::beginElapsedTime(QueryTarget::GPU, "GUI");
        ImGui::Render();
        QueryManager::endElapsedTime(QueryTarget::CPU, "GUI");
        QueryManager::endElapsedTime(QueryTarget::GPU, "GUI");
    }

    Screen::update();

    if (m_screenshotRequest)
    {
        m_screenshotRequest = false;
        takeScreenshot();
    }

    QueryManager::endElapsedTime(QueryTarget::CPU, "Total Time");
    QueryManager::endElapsedTime(QueryTarget::GPU, "Total Time");

    QueryManager::update();
}

void Engine::shutdown()
{
    m_game->quit();
    VoxelConeTracing::terminate();
    ImGui_ImplSdlGL3_Shutdown();
    SDL_Quit();
}

void Engine::registerCamera(ComponentPtr<CameraComponent> camera)
{
    m_cameras.push_back(camera);
}

void Engine::unregisterCamera(ComponentPtr<CameraComponent> camera)
{
    m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), camera), m_cameras.end());
}

void Engine::onQuit()
{
    m_running = false;
}

void Engine::onSDLEvent(SDL_Event& sdlEvent)
{
    ImGui_ImplSdlGL3_ProcessEvent(&sdlEvent);
}

void Engine::onWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
        resize(windowEvent.data1, windowEvent.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        m_paused = true;
        break;
    case SDL_WINDOWEVENT_RESTORED:
        m_paused = false;
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        m_paused = false;
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        m_paused = true;
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        m_paused = false;
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        m_paused = true;
        break;
    default: break;
    }
}

void Engine::receive(const QuitEvent&)
{
    m_running = false;
}

void Engine::resize(int width, int height)
{
    Screen::resize(width, height);

    for (auto& camera : m_cameras)
    {
        if (!camera)
            continue;

        camera->resize(float(width), float(height));
    }
}

void Engine::takeScreenshot()
{
    RGBA8* pixels = new RGBA8[Screen::getWidth() * Screen::getHeight()];

    glReadPixels(0, 0, static_cast<GLsizei>(Screen::getWidth()), static_cast<GLsizei>(Screen::getHeight()), GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    int channels = 4;

    // Invert_Y
    for (int i = 0; i * 2 < Screen::getHeight(); ++i)
    {
        int idx0 = i * Screen::getWidth();
        int idx1 = (Screen::getHeight() - 1 - i) * Screen::getWidth();
        for (int j = Screen::getWidth(); j > 0; --j)
            std::swap(pixels[idx0++], pixels[idx1++]);
    }

    std::string filename = "Screenshot" + std::to_string(m_screenshotCounter++) + ".png";
    if (!SOIL_save_image(filename.c_str(), SOIL_SAVE_TYPE_PNG, Screen::getWidth(), Screen::getHeight(), channels, reinterpret_cast<unsigned char*>(pixels)))
    {
        LOG("Failed to save screenshot.");
    }

    delete[] pixels;
}
