// NOTE: This file contains the code for the interpolation test

/*
  GameUpdateAndRender
  
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
 */

internal void DrawRectangle(f32 floatX, f32 floatY, f32 floatWidth, f32 floatHeight,
                            uint32 color, render_buffer buffer);

struct point
{
    f32 x;
    f32 y;
};

void DrawLine(point start, point end, uint32 color, render_buffer buffer)
{
    f32 dx = ABS(end.x - start.x);
    f32 dy = ABS(end.y - start.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;

    f32 thickness = 2.0f;
    f32 x, y, d;
    for(int i = 0; i < maxd; i++)
    {
        d = (f32)i*delta;

        x = ((1.0f-d)*start.x) + (d*end.x);
        y = ((1.0f-d)*start.y) + (d*end.y);
        
        DrawRectangle(x, y, thickness, thickness, color, buffer);
    }
}

void InterporateThreePoints(point p0, point p1, point p2, uint32 color, render_buffer buffer)
{
    f32 dx = ABS(p1.x - p0.x) + ABS(p2.x - p1.x);
    f32 dy = ABS(p1.y - p0.y) + ABS(p2.y - p1.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;

    f32 thickness = 2.0f;
    f32 x, y, d;
    for(int i = 0; i < maxd; i++)
    {
        d = (f32)i*delta;

        x = ((1.0f-d)*(1.0f-d)*(f32)p0.x) +
            (d*d*(f32)p2.x) +
            (2.0f*(1.0f-d)*d*(f32)p1.x);

        y = ((1.0f-d)*(1.0f-d)*(f32)p0.y) +
            (d*d*(f32)p2.y) +
            (2.0f*(1.0f-d)*d*(f32)p1.y);

        DrawRectangle(x, y, thickness, thickness, color, buffer);
    }
}

void InterporateFourPoints(point p0, point p1, point p2, point p3, uint32 color, render_buffer buffer)
{
    f32 dx = ABS(p1.x - p0.x) + ABS(p2.x - p1.x) + ABS(p2.x - p3.x);
    f32 dy = ABS(p1.y - p0.y) + ABS(p2.y - p1.y) + ABS(p2.y - p3.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;

    f32 thickness = 2.0f;
    f32 x, y, d;
    for(int i = 0; i < maxd; i++)
    {
        d = (f32)i*delta;

        x = ((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.x) +
            (d*d*d*(f32)p3.x) +
            (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.x) +
            (3.0f*(1.0f-d)*d*d*(f32)p2.x);

        y = ((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.y) +
            (d*d*d*(f32)p3.y) +
            (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.y) +
            (3.0f*(1.0f-d)*d*d*(f32)p2.y);

        DrawRectangle(x, y, thickness, thickness, color, buffer);
    }
}
