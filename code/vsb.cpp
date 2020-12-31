#include "defines.h"
#include "vsb.h"


internal void RenderWierdGradient(render_buffer buffer, int redOffset, int greenOffset)
{
    uint32 *pixel = (uint32 *)buffer.memory;
    for(int y = 0; y < buffer.height; y++)
    {
        for(int x = 0; x < buffer.width; x++)
        {
            *pixel++ = VSB_RGB(redOffset + x, greenOffset + y, 0);
        }
    }
}

void GameUpdateAndRender(game_memory *gameMemory)
{
    // TODO: we use the allocated memory like this, FOR NOW!
    game_state *gameState = (game_state *)gameMemory->memory;
    if(!gameMemory->initialized)
    {
        // NOTE: game state and stuff initialization
        gameMemory->initialized = true;
    }

    RenderWierdGradient(gameMemory->backBuffer, (int)gameState->redOffset, (int)gameState->greenOffset);
}
