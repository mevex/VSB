#pragma once

#include <math.h>
#include "defines.h"

// TODO: Intrinsics?
inline int32 RoundFloatToInt32(f32 value)
{
    return (int32)lroundf(value);
}

inline int32 TruncateFloatToInt32(f32 value)
{
    return (int32)(value);
}

union v2
{
    struct
    {
        f32 x, y;
    };
    f32 comp[2];

    v2(f32 x, f32 y)
    {
        this->x = x;
        this->y = y;
    }

    v2(){};
};

inline v2 operator+(v2 a, v2 b)
{
    return v2(a.x+b.x, a.y+b.y);
}

inline v2 operator+=(v2 &a, v2 b)
{
    a = a + b;
    return a;
}

inline v2 operator-(v2 a, v2 b)
{
    return v2(a.x-b.x, a.y-b.y);
}

inline v2 operator-(v2 v)
{
    return v2(-v.x, -v.y);
}

inline v2 operator*(v2 v, f32 value)
{
    return v2(v.x*value, v.y*value);
}

inline v2 operator*(f32 value, v2 v)
{
    return v * value;
}

inline v2 operator*=(v2 &v, f32 value)
{
    v = v * value;
    return v;
}
