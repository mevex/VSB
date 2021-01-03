#pragma once

// NOTE: Services that the platform layer provides to the game

// NOTE: These file I/O functions are NOT safe and are meant to be
// used only for debugging purposes during development.
struct debug_file
{
    uint32 size;
    void *memory;
};
void DebugFreeFile(debug_file *file);
debug_file DebugReadFile(char *fileName);
bool DebugWriteFile(char *filename, uint32 fileSize, void *memory);

// NOTE: Services that the game provides to the platform layer

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
};

struct game_state
{
    // TODO: maybe add frame time here?
    // TODO: these are just dummy values
    f32 redOffset, greenOffset;
    int a, b;
};

void GameUpdateAndRender(game_memory *gameMemory, game_input *input);
