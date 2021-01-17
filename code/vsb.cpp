#include "vsb.h"

#include "test_interpolation.cpp"

bmp_image LoadBMP(debug_read_file *ReadFile, char *filename)
{
    bmp_image result = {};

    debug_file bmpFile = ReadFile(filename);
    if(bmpFile.size)
    {
        bmp_header header = *(bmp_header *)bmpFile.memory;
        Assert(header.compression == 3);
        Assert(header.bitsPerPixel == 32);

        result.width = header.width;
        result.height = header.height;
        result.pixels = (byte *)bmpFile.memory + header.pixelOffsetInBytes;

        int redShift = FindFirstLowBitSet(header.redMask);
        int greenShift = FindFirstLowBitSet(header.greenMask);
        int blueShift = FindFirstLowBitSet(header.blueMask);
        int alphaShift = FindFirstLowBitSet(header.alphaMask);

        uint32 *pixel = (uint32 *)result.pixels;
        uint8 a, r, g, b;
        for(int index = 0; index < (result.width*result.height); index++)
        {
            r = (*pixel >> redShift) & 0xFF;
            g = (*pixel >> greenShift) & 0xFF;
            b = (*pixel >> blueShift) & 0xFF;
            a = (*pixel >> alphaShift) & 0xFF;

            *pixel++ = VSB_RGBA(r,g,b,a);
        }
    }

    return result;
}

void DrawBitmap(v2 p, v2 align, bmp_image bmp, render_buffer buffer)
{
    int left = RoundFloatToInt32(p.x - align.x);
    int top = RoundFloatToInt32(p.y - align.y);
    int width = bmp.width;
    int height = bmp.height;

    int sourceOffsetX = 0;
    int sourceOffsetY = 0;

    if(left < 0)
    {
        width += left;
        sourceOffsetX -= left;
        left = 0;
    }
    if(top < 0)
    {
        height += top;
        sourceOffsetY -= top;
        top = 0;
    }

    width = ((left + width) > buffer.width) ? (buffer.width - left) : width;
    height = ((top + height) > buffer.height) ? (buffer.height - top) : height;

    uint32 *destRow = (uint32 *)buffer.memory + buffer.width*top + left;
    uint32 *sourceRow = (uint32 *)bmp.pixels + (bmp.width*(bmp.height - 1));
    sourceRow += -(bmp.width*sourceOffsetY) + sourceOffsetX;
    f32 sr, sg, sb, a;
    f32 dr, dg, db;
    f32 w0, w1;
    for(int y = 0; y < height; y++)
    {
        uint32 *dest = destRow;
        uint32 *source = sourceRow;
        for(int x = 0; x < width; x++)
        {
            sr = (f32)((*source >> 16) & 0xFF);
            sg = (f32)((*source >> 8) & 0xFF);
            sb = (f32)((*source >> 0) & 0xFF);

            a = (f32)((*source >> 24) & 0xFF);
            //if(a < 1.0f)
            {
                w1 = a/255.0f;
                w0 = 1.0f - w1;

                dr = (f32)((*dest >> 16) & 0xFF);
                dg = (f32)((*dest >> 8) & 0xFF);
                db = (f32)((*dest >> 0) & 0xFF);

                sr = w0*dr + w1*sr;
                sg = w0*dg + w1*sg;
                sb = w0*db + w1*sb;
            }

            *dest = VSB_RGB(sr, sg, sb);
            source++;
            dest++;
        }
        destRow += buffer.width;
        sourceRow -= bmp.width;
    }
}

void DrawBitmap(v2 p, bmp_image bmp, render_buffer buffer)
{
    DrawBitmap(p, v2(0, 0), bmp, buffer);
}

void FillEntireBuffer(render_buffer buffer, uint32 color)
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

void DrawRectangle(v2 p, v2 size, uint32 color, render_buffer buffer)
{
    int left = RoundFloatToInt32(p.x);
    int top = RoundFloatToInt32(p.y);
    int width = RoundFloatToInt32(size.x);
    int height = RoundFloatToInt32(size.y);

    if(left < 0)
    {
        width += left;
        left = 0;
    }
    if(top < 0)
    {
        height += top;
        top = 0;
    }

    width = ((left + width) > buffer.width) ? (buffer.width - left) : width;
    height = ((top + height) > buffer.height) ? (buffer.height - top) : height;

    uint32 *row = (uint32 *)buffer.memory + top*buffer.width + left;

    for(int y = 0; y < height; y++)
    {
        uint32 *pixel = row;
        for(int x = 0; x < width; x++)
        {
            *pixel++ = color;
        }
        row += buffer.width;
    }
}

void DrawPixel(v2 p, uint32 color, render_buffer buffer)
{
    int32 x = RoundFloatToInt32(p.x);
    int32 y = RoundFloatToInt32(p.y);
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
        gameState->backgroundBMP = LoadBMP(gameMemory->DebugReadFile, "test_background.bmp");
        gameState->playerFront[0] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_1.bmp");
        gameState->playerFront[1] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_2.bmp");
        gameState->playerFront[2] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_3.bmp");

        gameState->playerPos = {200.0f, 200.0f};

        gameMemory->initialized = true;
    }

    // TODO: make sure the input vectors are max length 1
    v2 playerAccel;
    f32 playerSpeed = 3000.0f;
    playerAccel += input->gamepad.leftStick;
    playerAccel *= playerSpeed;
    playerAccel -= 10.0f*gameState->playerVelocity; // Artificial friction

    // newPos = 1/2*a*t^2 + v*t + oldPos
    v2 newPlayerPos = (0.5f*playerAccel*Square(input->dTime) +
                       gameState->playerVelocity*input->dTime +
                       gameState->playerPos);
    gameState->playerPos = newPlayerPos;
    // newVel = a*t + oldVel
    gameState->playerVelocity = playerAccel*input->dTime + gameState->playerVelocity;

    // NOTE: Here i clear the buffer to make sure that there are no
    // problems with the rendering
    uint32 backgroundColor = VSB_RGB(255, 0, 255);
    FillEntireBuffer(gameMemory->backBuffer, backgroundColor);
    DrawBitmap(v2(10.0f, 10.0f), gameState->backgroundBMP, gameMemory->backBuffer);

    DrawRectangle(gameState->playerPos - v2(5.0f, 5.0f), v2(10.0f, 10.0f),
                  VSB_RGB(150,0,0), gameMemory->backBuffer);

    DrawBitmap(gameState->playerPos, v2(72.0f, 182.0f),
               gameState->playerFront[2], gameMemory->backBuffer);
    DrawBitmap(gameState->playerPos, v2(72.0f, 182.0f),
               gameState->playerFront[1], gameMemory->backBuffer);
    DrawBitmap(gameState->playerPos, v2(72.0f, 182.0f),
               gameState->playerFront[0], gameMemory->backBuffer);
}

