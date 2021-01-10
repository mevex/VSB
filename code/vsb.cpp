#include "defines.h"
#include "vsb.h"

#include "test_interpolation.cpp"

internal void FillEntireBuffer(render_buffer buffer, uint32 color)
{
    uint32 *pixel = (uint32 *)buffer.memory;
    for(int y = 0; y < buffer.height; y++)
    {
        for(int x = 0; x < buffer.width; x++)
        {
            *pixel++ = color;
        }
    }
}

internal void DrawRectangle(f32 floatX, f32 floatY, f32 floatWidth, f32 floatHeight,
                            uint32 color, render_buffer buffer)
{
    int iX = RoundFloatToInt32(floatX);
    int iY = RoundFloatToInt32(floatY);
    int iWidth = RoundFloatToInt32(floatWidth);
    int iHeight = RoundFloatToInt32(floatHeight);

    iWidth = ((iX + iWidth) > buffer.width) ? (buffer.width - iX) : iWidth;
    iHeight = ((iY + iHeight) > buffer.height) ? (buffer.height - iY) : iHeight;

    if(iX < 0)
    {
        iWidth += iX;
        iX = 0;
    }
    if(iY < 0)
    {
        iHeight += iY;
        iY = 0;
    }

    uint32 *pixel = (uint32 *)buffer.memory + iY*buffer.width + iX;

    for(int y = 0; y < iHeight; y++)
    {
        for(int x = 0; x < iWidth; x++)
        {
            *pixel++ = color;
        }
        pixel += buffer.width - iWidth;
    }
}

void DrawPixel(int x, int y, uint32 color, render_buffer buffer)
{
    uint32 *pixel = (uint32 *)buffer.memory + buffer.width*y + x;
    *pixel = color;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    // TODO: we use the allocated memory like this, FOR NOW!
    game_state *gameState = (game_state *)gameMemory->memory;
    if(!gameMemory->initialized)
    {
        // NOTE: game state and stuff initialization
        gameMemory->initialized = true;

        char *filename = __FILE__;
        debug_file file = gameMemory->DebugReadFile(filename);
        if(file.memory)
        {
            gameMemory->DebugWriteFile("test.out", file.size, file.memory);
            gameMemory->DebugFreeFile(&file);
        }
    }

    uint32 backgroundColor = VSB_RGB(200, 200, 200);
    FillEntireBuffer(gameMemory->backBuffer, backgroundColor);

    gameState->x1 += 10.0f * input->gamepad.leftStickX;
    gameState->y1 -= 10.0f * input->gamepad.leftStickY;
    gameState->x2 += 10.0f * input->gamepad.rightStickX;
    gameState->y2 -= 10.0f * input->gamepad.rightStickY;

    gameState->x0 += 10.0f * input->keyboard.leftStickX;
    gameState->y0 -= 10.0f * input->keyboard.leftStickY;
    gameState->x3 += 10.0f * input->keyboard.rightStickX;
    gameState->y3 -= 10.0f * input->keyboard.rightStickY;

    point p0 = {10.0f + gameState->x0, 10.0f + gameState->y0};
    point p1 = {400.0f + gameState->x1, 400.0f + gameState->y1};
    point p2 = {600.0f + gameState->x2, 600.0f + gameState->y2};
    point p3 = {1200.0f + gameState->x3, 700.0f + gameState->y3};

    DrawLine(p0, p1, VSB_RGB(100, 0, 0), gameMemory->backBuffer);
    DrawLine(p2, p3, VSB_RGB(100, 0, 0), gameMemory->backBuffer);
    InterporateFourPoints(p0, p1, p2, p3, VSB_RGB(0,0,255), gameMemory->backBuffer);
    DrawRectangle(p0.x-5, p0.y-5, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawRectangle(p1.x-5, p1.y-5, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawRectangle(p2.x-5, p2.y-5, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawRectangle(p3.x-5, p3.y-5, 10, 10, VSB_RGB(150,0,0), gameMemory->backBuffer);
}
