// NOTE: This file contains the code for the interpolation test

void DrawRectangle(v2 p, v2 size, ui32 color, render_buffer buffer);

void DrawLine(v2 start, v2 end, ui32 color, render_buffer buffer)
{
    f32 dx = Abs(end.x - start.x);
    f32 dy = Abs(end.y - start.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;
    
    v2 p;
    v2 thickness(2.0f, 2.0f);
    f32 d;
    for(i32 i = 0; i < maxd; i++)
    {
        d = (f32)i*delta;
        
        p.x = ((1.0f-d)*start.x) + (d*end.x);
        p.y = ((1.0f-d)*start.y) + (d*end.y);
        
        DrawRectangle(p, thickness, color, buffer);
    }
}

void InterporateThreePoints(v2 p0, v2 p1, v2 p2, ui32 color, render_buffer buffer)
{
    f32 dx = Abs(p1.x - p0.x) + Abs(p2.x - p1.x);
    f32 dy = Abs(p1.y - p0.y) + Abs(p2.y - p1.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;
    
    v2 p;
    v2 thickness(2.0f, 2.0f);
    f32 d;
    for(i32 i = 0; i < maxd; i++)
    {
        d = (f32)i*delta;
        
        p.x = ((1.0f-d)*(1.0f-d)*(f32)p0.x) +
            (d*d*(f32)p2.x) +
            (2.0f*(1.0f-d)*d*(f32)p1.x);
        
        p.y = ((1.0f-d)*(1.0f-d)*(f32)p0.y) +
            (d*d*(f32)p2.y) +
            (2.0f*(1.0f-d)*d*(f32)p1.y);
        
        DrawRectangle(p, thickness, color, buffer);
    }
}

void InterporateFourPoints(v2 p0, v2 p1, v2 p2, v2 p3, ui32 color, render_buffer buffer)
{
    f32 dx = Abs(p1.x - p0.x) + Abs(p2.x - p1.x) + Abs(p2.x - p3.x);
    f32 dy = Abs(p1.y - p0.y) + Abs(p2.y - p1.y) + Abs(p2.y - p3.y);
    f32 maxd = Max(dx, dy);
    f32 delta = 1.0f / (f32)maxd;
    
    v2 p;
    v2 thickness(2.0f, 2.0f);
    f32 d;
    for(i32 i = 0; i < maxd; i++)
    {
        d = (f32)i*delta;
        
        p.x = ((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.x) +
            (d*d*d*(f32)p3.x) +
            (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.x) +
            (3.0f*(1.0f-d)*d*d*(f32)p2.x);
        
        p.y = ((1.0f-d)*(1.0f-d)*(1.0f-d)*(f32)p0.y) +
            (d*d*d*(f32)p3.y) +
            (3.0f*(1.0f-d)*(1.0f-d)*d*(f32)p1.y) +
            (3.0f*(1.0f-d)*d*d*(f32)p2.y);
        
        DrawRectangle(p, thickness, color, buffer);
    }
}
