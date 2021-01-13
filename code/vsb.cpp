#include "defines.h"
#include "utils.h"

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

void DrawBitmap(f32 fX, f32 fY, int xAlign, int yAlign, bmp_image bmp, render_buffer buffer)
{
    int iX = RoundFloatToInt32(fX) - xAlign;
    int iY = RoundFloatToInt32(fY) - yAlign;
    int width = bmp.width;
    int height = bmp.height;

    int sourceOffsetX = 0;
    int sourceOffsetY = 0;

    if(iX < 0)
    {
        width += iX;
        sourceOffsetX -= iX;
        iX = 0;
    }
    if(iY < 0)
    {
        height += iY;
        sourceOffsetY -= iY;
        iY = 0;
    }

    width = ((iX + width) > buffer.width) ? (buffer.width - iX) : width;
    height = ((iY + height) > buffer.height) ? (buffer.height - iY) : height;

    uint32 *destRow = (uint32 *)buffer.memory + buffer.width*iY + iX;
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

void DrawBitmap(f32 fX, f32 fY, bmp_image bmp, render_buffer buffer)
{
    DrawBitmap(fX, fY, 0, 0, bmp, buffer);
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

void DrawRectangle(f32 floatX, f32 floatY, f32 floatWidth, f32 floatHeight,
                            uint32 color, render_buffer buffer)
{
    int iX = RoundFloatToInt32(floatX);
    int iY = RoundFloatToInt32(floatY);
    int iWidth = RoundFloatToInt32(floatWidth);
    int iHeight = RoundFloatToInt32(floatHeight);

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

    iWidth = ((iX + iWidth) > buffer.width) ? (buffer.width - iX) : iWidth;
    iHeight = ((iY + iHeight) > buffer.height) ? (buffer.height - iY) : iHeight;

    uint32 *row = (uint32 *)buffer.memory + iY*buffer.width + iX;

    for(int y = 0; y < iHeight; y++)
    {
        uint32 *pixel = row;
        for(int x = 0; x < iWidth; x++)
        {
            *pixel++ = color;
        }
        row += buffer.width;
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
        gameState->backgroundBMP = LoadBMP(gameMemory->DebugReadFile, "test_background.bmp");
        gameState->playerFront[0] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_1.bmp");
        gameState->playerFront[1] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_2.bmp");
        gameState->playerFront[2] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_3.bmp");

        gameState->playerX = 100.0f;
        gameState->playerY = 100.0f;

        gameMemory->initialized = true;
    }

    gameState->playerX += 4.0f*input->gamepad.leftStickX;
    gameState->playerY -= 4.0f*input->gamepad.leftStickY;

    gameState->playerX += 4.0f*input->keyboard.leftStickX;
    gameState->playerY -= 4.0f*input->keyboard.leftStickY;

    uint32 backgroundColor = VSB_RGB(255, 0, 255);
    FillEntireBuffer(gameMemory->backBuffer, backgroundColor);
    DrawBitmap(10, 10, gameState->backgroundBMP, gameMemory->backBuffer);

    DrawRectangle(gameState->playerX - 5.0f, gameState->playerY - 5.0f, 10, 10,
                  VSB_RGB(150,0,0), gameMemory->backBuffer);

    DrawBitmap(gameState->playerX, gameState->playerY, 72, 182,
               gameState->playerFront[2], gameMemory->backBuffer);
    DrawBitmap(gameState->playerX, gameState->playerY, 72, 182,
               gameState->playerFront[1], gameMemory->backBuffer);
    DrawBitmap(gameState->playerX, gameState->playerY, 72, 182,
               gameState->playerFront[0], gameMemory->backBuffer);
}

