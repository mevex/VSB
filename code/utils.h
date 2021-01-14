// NOTE: This is a collection of utility functions
#pragma once

#include <intrin.h>

inline uint32 FindFirstLowBitSet(int32 value)
{
    unsigned long result;
    uint8 found = _BitScanForward(&result, value);
    return (found) ? result: -1;
}
