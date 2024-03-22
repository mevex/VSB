#pragma once

#include "utils.h"
#include "vsb_math.h"

// NOTE: Services that the platform layer provides to the game
//

// IMPORTANT: These file I/O functions are NOT safe and are meant to
// be used only for debugging purposes during development.
struct debug_file
{
    ui32 size;
    void *memory;
};

#if VSB_DEBUG

#define DEBUG_FREE_FILE(name) void name(debug_file *file)
typedef DEBUG_FREE_FILE(debug_free_file);

#define DEBUG_READ_FILE(name) debug_file name(char *filename)
typedef DEBUG_READ_FILE(debug_read_file);

#define DEBUG_WRITE_FILE(name) bool name(char *filename, ui32 fileSize, void *memory)
typedef DEBUG_WRITE_FILE(debug_write_file);

#endif

// NOTE: Services that the game provides to the platform layer
//

struct controller
{
    union
    {
        bool buttons[16];
        struct
        {
            bool up;
            bool down;
            bool left;
            bool right;
            
            bool a;
            bool b;
            bool x;
            bool y;
            
            bool leftBumper;
            bool rightBumper;
            bool leftTrigger;
            bool rightTrigger;
            
            bool menu; // Start
            bool view; // Select/Back
            bool leftThumb;
            bool rightThumb;
        };
    };
    
    f32 leftTriggerValue;
    f32 rightTriggerValue;
    
    // TODO Vectorize this
    v2 leftStick;
    v2 rightStick;
    
    bool connected;
};

struct game_input
{
    controller gamepad;
    controller keyboard;
    
    f32 dTime;
    bool analog;
};

struct render_buffer
{
    void *memory;
    i32 width;
    i32 height;
};

struct game_memory
{
    render_buffer backBuffer;
    
    ui64 memorySize;
    void* memory;
    
    bool initialized;
    
    debug_free_file *DebugFreeFile;
    debug_read_file *DebugReadFile;
    debug_write_file *DebugWriteFile;
};

#pragma pack(push, 1)
struct bmp_header
{
    i16 id;
    i32 size;
    i32 reserved;
    i32 pixelOffsetInBytes;
    
    i32 dibHeaderSize;
    i32 width;
    i32 height;
    i16 planes;
    i16 bitsPerPixel;
    i32 compression;
    i32 sizeOfBitmapData;
    i32 horizontalRes;
    i32 verticalRes;
    i32 paletteColors;
    i32 importantColors;
    
    i32 redMask;
    i32 greenMask;
    i32 blueMask;
    i32 alphaMask;
};

struct png_header
{
    byte i;
};
#pragma pack(pop)

struct bmp_image
{
    i32 width;
    i32 height;
    void* pixels;
    
    i32 xAlign;
    i32 yAlign;
};

#if 0
struct png_image
{
    void* voidness;
};
#endif

struct tile_coordinates
{
    i32 roomX;
    i32 roomY;
    
    i32 tileX;
    i32 tileY;
    
    v2 onTilePos;
};

struct room
{
    i32 roomX;
    i32 roomY;
    
    i32 horTiles;
    i32 verTiles;
    v2 mapShift; // -20,0
    
    // TODO change tile with a more appropriate "object", entity
    // maybe? Or maybe use both tiles and entities
    ui32 *tiles;
};

struct level
{
    f32 tileSidePixels;
    f32 tileSideMeters;
    f32 pixelsPerMeter;
    
    i32 horRooms;
    i32 verRooms;
    i32 nRooms;
    room rooms[2][2];
};

typedef enum particleColor
{
    red,
    green,
    blue,
    pink,
    yellow,
    orange,
    
    COUNT,
}
particleColor;

struct particle
{
    particleColor color;
    v2 position;
    v2 velocity;
};

#include <ctime>
#include <cstdlib>
#include <immintrin.h>

inline f32 GetRand()
{
    return f32(f32(rand()) / f32(RAND_MAX));
}

inline f32 GetRandInRange(f32 min, f32 max)
{
    return min + GetRand()*(max-min);
}

#define N_PARTICLES 10
struct game_state
{
#if 1
    bmp_image backgroundBMP;
    bmp_image playerFront[3];
    bmp_image testBMP;
    
    v2 playerVelocity;
    level level;
    tile_coordinates playerTilePos;
#endif
    
    v2 p0Pos;
    
    particle particles[N_PARTICLES];
    f32 forceMatrix[particleColor::COUNT][particleColor::COUNT];
    
    f32 maxRange;
    f32 minRange;
    f32 baseRepulsiveForce;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *gameMemory, game_input *input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
