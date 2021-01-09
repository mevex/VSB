// NOTE: This file contains the code for the interpolation test

internal void DrawRectangle(f32 floatX, f32 floatY, f32 floatWidth, f32 floatHeight,
                            uint32 color, render_buffer buffer);

struct point
{
    f32 x;
    f32 y;
};

void InterporateThreePoints(point p0, point p1, point p2, uint32 color, render_buffer buffer)
{
    f32 dx = ABS(p1.x - p0.x) + ABS(p2.x - p1.x);
    f32 dy = ABS(p1.y - p0.y) + ABS(p2.y - p1.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;

    f32 thickness = 2.0f;
    f32 x, y;
    point p = {};
    for(int i = 0; i < maxd; i++)
    {
        f32 d = (f32)i*delta;

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
    f32 x, y;
    for(int i = 0; i < maxd; i++)
    {
        f32 d = (f32)i*delta;

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
