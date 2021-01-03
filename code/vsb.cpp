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

internal void DrawRectangle(int xPos, int yPos, int width, int height,
                            uint32 color, render_buffer buffer)
{
    width = ((xPos + width) <= buffer.width) ? width : buffer.width - xPos;
    height = ((yPos + height) < buffer.height) ? height : buffer.height - yPos - 1;
    uint32 *pixel = (uint32 *)buffer.memory + buffer.width*yPos + xPos;

    for(int y = 0; y < height; y++)
    {
        pixel += buffer.width;
        for(int x = 0; x < width; x++)
        {
            pixel[x] = color;
        }
    }
}

void GameUpdateAndRender(game_memory *gameMemory, game_input *input)
{
    // TODO: we use the allocated memory like this, FOR NOW!
    game_state *gameState = (game_state *)gameMemory->memory;
    if(!gameMemory->initialized)
    {
        // NOTE: game state and stuff initialization
        gameMemory->initialized = true;

        gameState->a = 100;
        gameState->b = 100;

        char *filename = __FILE__;
        debug_file file = DebugReadFile(filename);
        if(file.memory)
        {
            DebugWriteFile("test.out", file.size, file.memory);
            DebugFreeFile(&file);
        }
    }

    gameState->redOffset += 0.5f;
    gameState->greenOffset += 0.5f;
    RenderWierdGradient(gameMemory->backBuffer, (int)gameState->redOffset, (int)gameState->greenOffset);

    gameState->a += (int)(4.0f * input->gamepad.leftStickX);
    gameState->b -= (int)(4.0f * input->gamepad.leftStickY);
    DrawRectangle(0, 0, 100, 100, VSB_RGB(0,0,0), gameMemory->backBuffer);
}
