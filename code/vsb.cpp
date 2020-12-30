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

void GameUpdateAndRender(render_buffer buffer, f32 redOffset, f32 greenOffset)
{
    RenderWierdGradient(buffer, (int)redOffset, (int)greenOffset);
}
