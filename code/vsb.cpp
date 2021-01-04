#include "defines.h"
#include "vsb.h"


internal void RenderWierdGradient(render_buffer buffer, int redOffset, int greenOffset)
{
    uint32 *pixel = (uint32 *)buffer.memory;
    for(int y = 0; y < buffer.height; y++)
    {
        for(int x = 0; x < buffer.width; x++)
        {
            *pixel++ = VSB_RGB(200,200,200);//VSB_RGB(redOffset + x, greenOffset + y, 0);
        }
    }
}

internal void DrawRectangle(int xPos, int yPos, int width, int height,
                            uint32 color, render_buffer buffer)
{
/*    xPos = xPos >= 0 ? xPos : 0;
    yPos = yPos >= 0 ? yPos : 0;
    width = ((xPos + width) <= buffer.width) ? width : buffer.width - xPos;
    height = ((yPos + height) < buffer.height) ? height : buffer.height - yPos - 1; */

    uint32 *pixel = (uint32 *)buffer.memory;

    for(int y = yPos; y < (height + yPos); y++)
    {
        for(int x = xPos; x < (width + xPos); x++)
        {
            if(x > 0 && x < 1280 &&
               y > 0 && y < 720)
            {
                pixel[y*buffer.width + x] = color;
            }
        }
    }
}

struct point
{
    int x;
    int y;
};

int Max(int a, int b)
{
    if(a > b)
        return a;
    else
        return b;
}

void DrawPixel(int x, int y, uint32 color, render_buffer buffer)
{
    uint32 *pixel = (uint32 *)buffer.memory + buffer.width*y + x;
    *pixel = color;
}

void InterporateThreePoints(point p0, point p1, point p2, uint32 color, render_buffer buffer)
{
    int dx = p2.x - p0.x;
    int dy = p2.y - p0.y;
    int maxd = Max(dx, dy) * 2;
    f32 delta = 1.0f / (f32)maxd;

    int thickness = 2;
    point p = {};
    for(int i = 0; i < maxd; i++)
    {
        f32 d = (f32)i*delta;

        p.x = (int)(((1.0f-d)*(1.0f-d)*(f32)p0.x) +
                    (d*d*(f32)p2.x) +
                    (2.0f*(1.0f-d)*d*(f32)p1.x));

        p.y = (int)(((1.0f-d)*(1.0f-d)*(f32)p0.y) +
                    (d*d*(f32)p2.y) +
                    (2.0f*(1.0f-d)*d*(f32)p1.y));
        DrawRectangle(p.x, p.y, thickness, thickness, color, buffer);
    }
}

void InterporateFourPoints(point p0, point p1, point p2, point p3, uint32 color, render_buffer buffer)
{
    int dx = p3.x - p0.x;
    int dy = p3.y - p0.y;
    int maxd = Max(dx, dy) * 2;
    f32 delta = 1.0f / (f32)maxd;

    int thickness = 2;
    point p = {};
    for(int i = 0; i < maxd; i++)
    {
        f32 d = (f32)i*delta;

        p.x = (int)(((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.x) +
                    (d*d*d*(f32)p3.x) +
                    (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.x) +
                    (3.0f*(1.0f-d)*d*d*(f32)p2.x));

        p.y = (int)(((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.y) +
                    (d*d*d*(f32)p3.y) +
                    (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.y) +
                    (3.0f*(1.0f-d)*d*d*(f32)p2.y));

        DrawRectangle(p.x, p.y, thickness, thickness, color, buffer);
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

    gameState->x1 += (int)(2.0f * input->gamepad.leftStickX);
    gameState->y1 -= (int)(2.0f * input->gamepad.leftStickY);
    gameState->x2 += (int)(2.0f * input->gamepad.rightStickX);
    gameState->y2 -= (int)(2.0f * input->gamepad.rightStickY);

    gameState->x0 += (int)(2.0f * input->keyboard.leftStickX);
    gameState->y0 -= (int)(2.0f * input->keyboard.leftStickY);
    gameState->x3 += (int)(2.0f * input->keyboard.rightStickX);
    gameState->y3 -= (int)(2.0f * input->keyboard.rightStickY);

    point p0 = {10 + gameState->x0, 10 + gameState->y0};
    point p1 = {400 + gameState->x1, 400 + gameState->y1};
    point p2 = {600 + gameState->x2, 600 + gameState->y2};
    point p3 = {1200 + gameState->x3, 700 + gameState->y3};

    DrawRectangle(p0.x, p0.y, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawRectangle(p1.x, p1.y, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawRectangle(p2.x, p2.y, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawRectangle(p3.x, p3.y, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    InterporateFourPoints(p0, p1, p2, p3, VSB_RGB(0,0,255), gameMemory->backBuffer);
}
