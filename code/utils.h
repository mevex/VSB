// NOTE: This is a collection of utility functions
#pragma once

#include <intrin.h>
#include "defines.h"

inline ui32 FindFirstLowBitSet(i32 value)
{
    unsigned long result;
    _BitScanForward(&result, value);
    return result;
}

template <typename T>
inline void Swap(T &a, T &b)
{
    T temp = a;
    a = b;
    b = temp;
}

inline bool AlmostZero(f32 value)
{
    return value < ZERO && value > -ZERO;
}

inline bool AlmostZero(f32 value, f32 zero)
{
    return value < zero && value > -zero;
}