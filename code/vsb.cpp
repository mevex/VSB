#include "vsb.h"

//#include "test_interpolation.cpp"

void DrawPixel(v2 p, ui32 color, render_buffer buffer)
{
    if(p.x < 0 || p.x >= buffer.width ||
       p.y < 0 || p.y >= buffer.height)
        return;
    
    // TODO(mevex): This are rounds for now (Research needed)
    i32 x = RoundToInt32(p.x);
    i32 y = RoundToInt32(p.y);
    
    ui32 *targetPixel = (ui32 *)buffer.memory + buffer.width*y + x;
    f32 alpha = ExtractA(color) / 255.0f;
    
    if(alpha < 1.0f)
    {
        ui8 rInput = ExtractR(color);
        ui8 gInput = ExtractG(color);
        ui8 bInput = ExtractB(color);
        
        ui8 rTarget = ExtractR(*targetPixel);
        ui8 gTarget = ExtractG(*targetPixel);
        ui8 bTarget = ExtractB(*targetPixel);
        
        ui8 rNew = (ui8)(RoundToInt32(alpha*rInput + (1-alpha)*rTarget));
        ui8 gNew = (ui8)(RoundToInt32(alpha*gInput + (1-alpha)*gTarget));
        ui8 bNew = (ui8)(RoundToInt32(alpha*bInput + (1-alpha)*bTarget));
        
        color = VSB_RGBA(rNew, gNew, bNew, 0xff);
    }
    
    *targetPixel = color;
    
}

void DrawLine(v2 beg, v2 end, ui32 color, render_buffer buffer)
{
    // NOTE(mevex): This is an implementation of Xiaolin Wu's line algorithm
    
    bool steep = Abs(end.y - beg.y) > Abs(end.x - beg.x);
    if(steep)
    {
        Swap(beg.x, beg.y);
        Swap(end.x, end.y);
    }
    if(beg.x > end.x)
    {
        Swap(beg.x, end.x);
        Swap(beg.y, end.y);
    }
    
    f32 dx = end.x - beg.x;
    f32 dy = end.y - beg.y;
    f32 gradient = AlmostZero(dx) ? 1.0f : dy/dx;
    
    ui8 r = ExtractR(color);
    ui8 g = ExtractG(color);
    ui8 b = ExtractB(color);
    ui8 a = ExtractA(color);
    
    float y = beg.y;
    if(steep)
    {
        for(i32 x = RoundToInt32(beg.x); x <= RoundToInt32(end.x); ++x)
        {
            DrawPixel(v2(y - 0.5f, x), VSB_RGBA(r, g, b, ReverseFractionalPart(y)*a), buffer);
            DrawPixel(v2(y + 0.5f, x), VSB_RGBA(r, g, b, FractionalPart(y)*a), buffer);
            y += gradient;
        }
    }
    else
    {
        for(i32 x = RoundToInt32(beg.x); x <= RoundToInt32(end.x); ++x)
        {
            DrawPixel(v2(x, y - 0.5f), VSB_RGBA(r, g, b, ReverseFractionalPart(y)*a), buffer);
            DrawPixel(v2(x, y + 0.5f), VSB_RGBA(r, g, b, FractionalPart(y)*a), buffer);
            y += gradient;
        }
    }
}

void DrawRectangle(v2 p, v2 size, ui32 color, render_buffer buffer)
{
    i32 left = RoundToInt32(p.x);
    i32 top = RoundToInt32(p.y);
    i32 width = RoundToInt32(size.x);
    i32 height = RoundToInt32(size.y);
    
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
    
    ui32 *row = (ui32 *)buffer.memory + top*buffer.width + left;
    
    for(i32 y = 0; y < height; y++)
    {
        ui32 *pixel = row;
        for(i32 x = 0; x < width; x++)
        {
            *pixel++ = color;
        }
        row += buffer.width;
    }
}

void DrawFilledCircle(v2 p, f32 r, ui32 color, render_buffer buffer)
{
    f32 fade = 0.5;
    v2 LowerLeftBound = v2(p.x-r, p.y-r);
    v2 UpperRightBound = v2(p.x+r, p.y+r);
    
    for(i32 y = FloorToInt32(LowerLeftBound.y) - 1; y < CeilingToInt32(UpperRightBound.y) + 1; y++)
    {
        for(i32 x = FloorToInt32(LowerLeftBound.x) - 1; x < CeilingToInt32(UpperRightBound.x) + 1; x++)
        {
            v2 distanceVector = v2(x,y) - p;
            f32 distance = SquareRoot(InnerProduct(distanceVector, distanceVector));
            if(distance < (r + fade))
            {
                if(distance < r)
                {
                    DrawPixel(v2(x,y), color, buffer);
                }
                else
                {
                    f32 delta = (distance - r);
                    f32 floatAlpha = (fade - delta) / fade;
                    
                    ui8 alpha = ui8(RoundToInt32(floatAlpha * 255.0f));
                    ui8 red   = ExtractR(color);
                    ui8 green = ExtractG(color);
                    ui8 blue  = ExtractB(color);
                    
                    ui32 newColor = VSB_RGBA(red, green, blue, alpha);
                    DrawPixel(v2(x,y), newColor, buffer);
                }
            }
        }
    }
}

void FillEntireBuffer(render_buffer buffer, ui32 color)
{
    ui32 bytesPerPizel = 4;
    ui32 bufferBytes = buffer.width*buffer.height*bytesPerPizel;
    
    ui32 loopCycles = bufferBytes / 32;
    ui32 remainingPixels = (bufferBytes % 32) / bytesPerPizel;
    
    // NOTE(mevex): make sure that we have entire pixels left
    Assert(((bufferBytes % 32) % bytesPerPizel) == 0);
    
    __m256i c = _mm256_set1_epi32(color);
    __m256i *dest = (__m256i *)buffer.memory;
    for(ui32 i = 0; i < loopCycles; ++i)
    {
        _mm256_store_si256(dest, c);
        ++dest;
    }
    
    ui32 *remainingDest = (ui32 *)dest;
    for(ui32 i = 0; i < remainingPixels; ++i)
    {
        *remainingDest++ = color;
    }
}

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
        
        i32 redShift = FindFirstLowBitSet(header.redMask);
        i32 greenShift = FindFirstLowBitSet(header.greenMask);
        i32 blueShift = FindFirstLowBitSet(header.blueMask);
        i32 alphaShift = FindFirstLowBitSet(header.alphaMask);
        
        ui32 *pixel = (ui32 *)result.pixels;
        ui8 a, r, g, b;
        for(i32 index = 0; index < (result.width*result.height); index++)
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

void DrawBitmap(v2 p, v2 shift, bmp_image bmp, render_buffer buffer)
{
    // TODO(mevex): This is just for debugging, use vectors and consider "half pixels"
    // TODO(mevex): Shift is now y-going-down
    i32 left = RoundToInt32(p.x - shift.x);
    i32 bottom = RoundToInt32(p.y - shift.y);
    i32 width = bmp.width;
    i32 height = bmp.height;
    
    // NOTE(mevex): These are used if the bitmap is partially off the screen
    i32 sourceOffsetX = 0;
    i32 sourceOffsetY = 0;
    
    if(left < 0)
    {
        width += left;
        sourceOffsetX -= left;
        left = 0;
    }
    if(bottom < 0)
    {
        height += bottom;
        sourceOffsetY -= bottom;
        bottom= 0;
    }
    
    width = ((left + width) > buffer.width) ? (buffer.width - left) : width;
    height = ((bottom + height) > buffer.height) ? (buffer.height - bottom) : height;
    
    ui32 *destRow = (ui32 *)buffer.memory + buffer.width*bottom + left;
    ui32 *sourceRow = (ui32 *)bmp.pixels;
    sourceRow += (bmp.width*sourceOffsetY) + sourceOffsetX;
    f32 sr, sg, sb, a;  // source colors
    f32 dr, dg, db;     // destination colors
    f32 w0, w1;
    for(i32 y = 0; y < height; y++)
    {
        ui32 *dest = destRow;
        ui32 *source = sourceRow;
        for(i32 x = 0; x < width; x++)
        {
            sr = (f32)((*source >> 16) & 0xFF);
            sg = (f32)((*source >> 8) & 0xFF);
            sb = (f32)((*source >> 0) & 0xFF);
            
            a = (f32)((*source >> 24) & 0xFF);
            if(a < 255.0f)
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
        sourceRow += bmp.width;
    }
}

void DrawBitmap(v2 p, bmp_image bmp, render_buffer buffer)
{
    DrawBitmap(p, v2(0, 0), bmp, buffer);
}

inline v2 ConvertTileCoordinatesIntoV2(level *level, tile_coordinates p)
{
    v2 result = {};
    
    result.x += p.tileX*level->tileSidePixels;
    result.y += p.tileY*level->tileSidePixels;
    result += p.onTilePos*level->pixelsPerMeter;
    
    room room = level->rooms[p.roomY][p.roomX];
    result += room.mapShift;
    
    return result;
}

void DrawBitmap(level *level, tile_coordinates p, v2 align, bmp_image bmp, render_buffer buffer)
{
    v2 pos = ConvertTileCoordinatesIntoV2(level, p);
    DrawBitmap(pos, align, bmp, buffer);
}

void RectifyTileCoordinates(tile_coordinates *position, level *level)
{
    i32 tileDiffX = FloorToInt32(position->onTilePos.x / level->tileSideMeters);
    i32 tileDiffY = FloorToInt32(position->onTilePos.y / level->tileSideMeters);
    v2 newOnTilePos = {position->onTilePos.x - tileDiffX*level->tileSideMeters, position->onTilePos.y - tileDiffY*level->tileSideMeters};
    
    position->tileX += tileDiffX;
    position->tileY += tileDiffY;
    position->onTilePos = newOnTilePos;
    
    i32 roomX = position->roomX;
    i32 roomY = position->roomY;
    room testRoom = level->rooms[roomX][roomY];;
    
    if(position->tileX < 0)
    {
        testRoom = level->rooms[roomX-1][roomY];
        position->roomX--;
        position->tileX = Mod(position->tileX, testRoom.horTiles);
    }
    else if(position->tileX >= testRoom.horTiles)
    {
        testRoom = level->rooms[roomX+1][roomY];
        position->roomX++;
        position->tileX = Mod(position->tileX, testRoom.horTiles);
    }
    else if(position->tileY < 0)
    {
        position->roomY--;
        testRoom = level->rooms[roomX][roomY-1];
        position->tileY = Mod(position->tileY, testRoom.verTiles);
    }
    else if(position->tileY >= testRoom.verTiles)
    {
        position->roomY++;
        testRoom = level->rooms[roomX][roomY+1];
        position->tileY = Mod(position->tileY, testRoom.verTiles);
    }
    
}

tile_coordinates ScreenPosToTilePos(v2 position, level *level, i32 roomX = 0, i32 roomY = 0)
{
    tile_coordinates result = {};
    
    result.roomX = roomX;
    result.roomY = roomY;
    
    room testRoom = level->rooms[roomY][roomX];
    result.onTilePos = position/level->pixelsPerMeter - testRoom.mapShift/level->pixelsPerMeter;
    
    RectifyTileCoordinates(&result, level);
    
    return result;
}

i32 GetTileValue(level *level, tile_coordinates testP)
{
    i32 horTiles = level->rooms[testP.roomY][testP.roomX].horTiles;
    ui32 *tiles = level->rooms[testP.roomY][testP.roomX].tiles;
    i32 result = *(tiles + testP.tileY*horTiles + testP.tileX);
    return result;
}

void DrawTileRoom(level *level, tile_coordinates playerP, render_buffer buffer)
{
    ui32 colors[2] = {VSB_RGB(255,255,255), VSB_RGB(240,240,240)};
    ui32 black = VSB_RGB(20,20,20);
    room playerRoom = level->rooms[playerP.roomY][playerP.roomX];
    
    i32 colorIndex = 0;
    ui32 color = 0;
    for(i32 y = 0; y < playerRoom.verTiles; y++)
    {
        for(i32 x = 0; x < playerRoom.horTiles; x++)
        {
            v2 p = {(f32)(x)*level->tileSidePixels, (f32)(y)*level->tileSidePixels};
            p += playerRoom.mapShift;
            i32 tile = *(playerRoom.tiles + y*playerRoom.horTiles + x);
            if(tile == 1)
            {
                DrawRectangle(p, v2(level->tileSidePixels, level->tileSidePixels), black, buffer);
            }
            else
            {
                color = colors[colorIndex%2];
                DrawRectangle(p, v2(level->tileSidePixels, level->tileSidePixels), color, buffer);
            }
            colorIndex++;
        }
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    // TODO: we use the allocated memory like this, FOR NOW!
    game_state *gameState = (game_state *)gameMemory->memory;
    
    ui32 room00[9][15] =
    {
        {1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1,  1, 1, 1, 1, 0,  0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1,  1, 1, 0, 1, 1,  1, 1, 1, 1, 1}
    };
    
    ui32 room10[9][15] =
    {
        {1, 1, 1, 1, 1,  1, 1, 0, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    
    ui32 room01[9][15] =
    {
        {1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1,  1, 1, 0, 1, 1,  1, 1, 1, 1, 1}
    };
    
    ui32 room11[9][15] =
    {
        {1, 1, 1, 1, 1,  1, 1, 0, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    
    if(!gameMemory->initialized)
    {
        // NOTE: game state and stuff initialization
        gameState->backgroundBMP = LoadBMP(gameMemory->DebugReadFile, "test_background.bmp");
        gameState->playerFront[0] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_1.bmp");
        gameState->playerFront[1] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_2.bmp");
        gameState->playerFront[2] = LoadBMP(gameMemory->DebugReadFile, "test_player_front_3.bmp");
        
        gameState->level.tileSidePixels = 80.0f;
        gameState->level.tileSideMeters = 1.5f;
        gameState->level.pixelsPerMeter = gameState->level.tileSidePixels / gameState->level.tileSideMeters;
        
        gameState->level.horRooms = 2;
        gameState->level.verRooms = 2;
        gameState->level.nRooms = gameState->level.horRooms*gameState->level.verRooms;
        gameState->level.rooms[0][0].horTiles = 15;
        gameState->level.rooms[0][0].verTiles = 9;
        gameState->level.rooms[0][0].mapShift = v2(20, 0);
        gameState->level.rooms[0][0].tiles = (ui32 *)room00;
        
        gameState->level.rooms[0][1] = gameState->level.rooms[0][0];
        gameState->level.rooms[1][0] = gameState->level.rooms[0][0];
        gameState->level.rooms[1][1] = gameState->level.rooms[0][0];
        gameState->level.rooms[0][1].tiles = (ui32 *)room01;
        gameState->level.rooms[1][0].tiles = (ui32 *)room10;
        gameState->level.rooms[1][1].tiles = (ui32 *)room11;
        
        gameState->playerTilePos = ScreenPosToTilePos(v2(250, 250), &gameState->level, 0, 1);
        
        gameMemory->initialized = true;
    }
    // NOTE(mevex): These are in meters
    // TODO: make sure the input vectors are max length 1
    Assert(Abs(input->gamepad.leftStick.x) <= 1);
    Assert(Abs(input->gamepad.leftStick.y) <= 1);
    v2 playerAccel;
    f32 playerSpeed = 40.0f;
    playerAccel += input->gamepad.leftStick;
    playerAccel *= playerSpeed;
    playerAccel -= 10.0f*gameState->playerVelocity; // Artificial friction
    
    // newPos = 1/2*a*t^2 + v*t + oldPos
    tile_coordinates newPlayerTilePos = gameState->playerTilePos;
    newPlayerTilePos.onTilePos = (0.5f*playerAccel*Square(input->dTime) +
                                  gameState->playerVelocity*input->dTime +
                                  gameState->playerTilePos.onTilePos);
    
    // newVel = a*t + oldVel
    gameState->playerVelocity = playerAccel*input->dTime + gameState->playerVelocity;
    
    RectifyTileCoordinates(&newPlayerTilePos, &gameState->level);
    i32 tileValue = GetTileValue(&gameState->level, newPlayerTilePos);
    if(tileValue == 0)
    {
        gameState->playerTilePos = newPlayerTilePos;
    }
    else
    {
        gameState->playerVelocity = v2(0, 0);
    }
    
    // NOTE: Here i clear the buffer to make sure that there are no mistakes with the rendering
    ui32 backgroundColor = VSB_RGB(255, 0, 255);
    FillEntireBuffer(gameMemory->backBuffer, backgroundColor);
    DrawTileRoom(&gameState->level, gameState->playerTilePos, gameMemory->backBuffer);
    
    // TODO(mevex): Maybe make this "real" for debug build
    v2 currentTile = {gameState->playerTilePos.tileX * gameState->level.tileSidePixels, gameState->playerTilePos.tileY * gameState->level.tileSidePixels};
    // TODO(mevex): This is hardcoded
    currentTile += v2(20,0);
    DrawRectangle(currentTile, v2(gameState->level.tileSidePixels, gameState->level.tileSidePixels), VSB_RGB(255,200,200), gameMemory->backBuffer);
    
    v2 playerPos = ConvertTileCoordinatesIntoV2(&gameState->level, gameState->playerTilePos);
    DrawRectangle(playerPos - v2(5.0f, 5.0f), v2(10.0f, 10.0f), VSB_RGB(150,0,0), gameMemory->backBuffer);
    DrawBitmap(&gameState->level, gameState->playerTilePos, v2(72, 35), gameState->playerFront[2], gameMemory->backBuffer);
    DrawBitmap(&gameState->level, gameState->playerTilePos, v2(72, 35), gameState->playerFront[1], gameMemory->backBuffer);
    DrawBitmap(&gameState->level, gameState->playerTilePos, v2(72, 35), gameState->playerFront[0], gameMemory->backBuffer);
    //DrawBitmap(v2(200, 200), gameState->testBMP, gameMemory->backBuffer);
    
    DrawFilledCircle(playerPos, 20, VSB_RGB(255,0,0), gameMemory->backBuffer);
    DrawLine(v2(10, 10), v2(100, 31), VSB_RGB(0,255,0), gameMemory->backBuffer);
    DrawLine(v2(90, 10), v2(40, 310), VSB_RGB(0,255,0), gameMemory->backBuffer);
}
