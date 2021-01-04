// NOTE: This file contains the code for the interpolation test

internal void DrawRectangle(int xPos, int yPos, int width, int height,
                            uint32 color, render_buffer buffer);

struct point
{
    int x;
    int y;
};

void InterporateThreePoints(point p0, point p1, point p2, uint32 color, render_buffer buffer)
{
    int dx = ABS(p1.x - p0.x) + ABS(p2.x - p1.x);
    int dy = ABS(p1.y - p0.y) + ABS(p2.y - p1.y);
    int maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;

    int thickness = 2;
    point p = {};
    for(int i = 0; i < maxd; i++)
    {
        f32 d = (f32)i*delta;

        p.x = (int)(((1.0f-d)*(1.0f-d)*(f32)p0.x) +
                    (d*d*(f32)p2.x) +
                    (2.0f*(1.0f-d)*d*(f32)p1.x));

        p.y = (int)(((1.0f-d)*(1.0f-d)*(f32)p0.y) +
                    (d*d*(f32)p2.y) +
                    (2.0f*(1.0f-d)*d*(f32)p1.y));
        DrawRectangle(p.x, p.y, thickness, thickness, color, buffer);
    }
}

void InterporateFourPoints(point p0, point p1, point p2, point p3, uint32 color, render_buffer buffer)
{
    int dx = ABS(p1.x - p0.x) + ABS(p2.x - p1.x) + ABS(p2.x - p3.x);
    int dy = ABS(p1.y - p0.y) + ABS(p2.y - p1.y) + ABS(p2.y - p3.y);
    int maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;

    int thickness = 2;
    point p = {};
    for(int i = 0; i < maxd; i++)
    {
        f32 d = (f32)i*delta;

        p.x = (int)(((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.x) +
                    (d*d*d*(f32)p3.x) +
                    (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.x) +
                    (3.0f*(1.0f-d)*d*d*(f32)p2.x));

        p.y = (int)(((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.y) +
                    (d*d*d*(f32)p3.y) +
                    (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.y) +
                    (3.0f*(1.0f-d)*d*d*(f32)p2.y));

        DrawRectangle(p.x, p.y, thickness, thickness, color, buffer);
    }
}
