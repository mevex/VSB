#pragma once

// NOTE: Services that the platform layer provides to the game
//

// IMPORTANT: These file I/O functions are NOT safe and are meant to
// be used only for debugging purposes during development.
struct debug_file
{
    uint32 size;
    void *memory;
};

#if VSB_DEBUG

#define DEBUG_FREE_FILE(name) void name(debug_file *file)
typedef DEBUG_FREE_FILE(debug_free_file);

#define DEBUG_READ_FILE(name) debug_file name(char *filename)
typedef DEBUG_READ_FILE(debug_read_file);

#define DEBUG_WRITE_FILE(name) bool name(char *filename, uint32 fileSize, void *memory)
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
    f32 leftStickX;
    f32 leftStickY;
    f32 rightStickX;
    f32 rightStickY;

    bool connected;
};

struct game_input
{
    controller gamepad;
    controller keyboard;

    bool analog;
};

struct render_buffer
{
    void *memory;
    int width;
    int height;
};

struct game_memory
{
    bool initialized;

    render_buffer backBuffer;

    uint64 memorySize;
    void* memory;

    debug_free_file *DebugFreeFile;
    debug_read_file *DebugReadFile;
    debug_write_file *DebugWriteFile;
};

struct game_state
{
    // TODO: maybe add frame time here?
    // TODO: these are just dummy values
    f32 redOffset, greenOffset;
    int x0, x1, x2, x3;
    int y0, y1, y2, y3;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *gameMemory, game_input *input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
