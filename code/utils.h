// NOTE: This is a collection of utility functions
#pragma once

#include <intrin.h>

inline ui32 FindFirstLowBitSet(i32 value)
{
    unsigned long result;
    ui8 found = _BitScanForward(&result, value);
    return (found) ? result: -1;
}
