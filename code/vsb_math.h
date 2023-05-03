#pragma once

#include <math.h>
#include "defines.h"

#define Max(a, b) ((a)>(b)) ? (a) : (b)
#define Min(a, b) ((a)<(b)) ? (a) : (b)
#define Clamp(value, min, max) ((value)>(max)) ? (max) : (((value)<(min)) ? (min) : (value))

#define Abs(value) (((value)>=0) ? (value) : -(value))

// TODO: Intrinsics?
inline i32 RoundToInt32(f32 value)
{
    return (i32)lroundf(value);
}

inline i32 FloorToInt32(f32 value)
{
    return (i32)floorf(value);
}

inline i32 CeilingToInt32(f32 value)
{
    return FloorToInt32(value) + 1;
}

inline i32 TruncateFloatToInt32(f32 value)
{
    return (i32)(value);
}

inline f32 FractionalPart(f32 value)
{
    return value - f32(FloorToInt32(value));
}

inline f32 ReverseFractionalPart(f32 value)
{
    return 1 - FractionalPart(value);
}

inline i32 Square(i32 value)
{
    return value*value;
}

inline f32 Square(f32 value)
{
    return value*value;
}

inline f32 SquareRoot(f32 value)
{
    return f32(sqrt(value));
}

inline i32 Mod(int a, int b)
{
    if(b < 0) b = -b;
    i32 result = a % b;
    return result < 0 ? result + b: result;
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
    v2(i32 x, i32 y)
    {
        this->x = (f32)x;
        this->y = (f32)y;
    }
    v2(f32 x, i32 y)
    {
        this->x = x;
        this->y = (f32)y;
    }
    v2(i32 x, f32 y)
    {
        this->x = (f32)x;
        this->y = y;
    }
    v2()
    {
        this->x = 0.0f;
        this->y = 0.0f;
    }
};

inline f32 InnerProduct(v2 a, v2 b)
{
    return a.x*b.x + a.y*b.y;
}

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

inline v2 operator-=(v2 &a, v2 b)
{
    a = a + (-b);
    return a;
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

inline v2 operator/(v2 v, f32 value)
{
    return v2(v.x/value, v.y/value);
}

inline v2 operator/=(v2 &v, f32 value)
{
    v = v / value;
    return v;
}
