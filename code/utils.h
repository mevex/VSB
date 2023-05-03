// NOTE: This is a collection of utility functions
#pragma once

#include <intrin.h>
#include "defines.h"

inline ui32 FindFirstLowBitSet(i32 value)
{
    unsigned long result;
    ui8 found = _BitScanForward(&result, value);
    return (found) ? result: -1;
}

template <typename T>
inline void Swap(T &a, T &b)
{
    T temp = a;
    a = b;
    b = temp;
}

bool AlmostZero(f32 value)
{
    return value < ZERO && value > -ZERO;
}