#pragma once

// NOTE: Services that the platform layer provides to the game



// NOTE: Services that the game provides to the platform layer

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
    f32 redOffset = 0;
    f32 greenOffset = 0;
};

// TODO: add input to the parameters
void GameUpdateAndRender(game_memory *gameMemory);
