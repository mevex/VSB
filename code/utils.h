// NOTE: This is a collection of utility functions
#pragma once

#include <math.h>

// TODO: Intrinsics?
inline int32 RoundFloatToInt32(f32 value)
{
    return (int32)lroundf(value);
}

inline int32 TruncateFloatToInt32(f32 value)
{
    return (int32)(value);
}

#include <intrin.h>
inline uint32 FindFirstLowBitSet(int32 value)
{
    unsigned long result;
    uint8 found = _BitScanForward(&result, value);
    return (found) ? result: -1;
}
