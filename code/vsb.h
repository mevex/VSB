#pragma once

struct render_buffer
{
    void *memory;
    int width;
    int height;
};

void GameUpdateAndRender(render_buffer buffer, f32 redOffset, f32 greenOffset);
