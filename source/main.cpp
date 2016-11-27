#include <engine/Engine.h>
#include <memory>
#include "game/VoxelConeTracingDemo/VoxelConeTracingDemo.h"

int main(int, char**)
{
    std::unique_ptr<Engine> engine = std::make_unique<Engine>();
    std::unique_ptr<VoxelConeTracingDemo> game = std::make_unique<VoxelConeTracingDemo>();

    engine->init(game.get());

    while (engine->running())
    {
        engine->update();
    }

    engine->shutdown();

    return 0;
}
