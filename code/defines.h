// NOTE: This is a collection of macros and typedefs widely used
// throughout the project

#pragma once
#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float f32;
typedef double f64;

typedef uint8 byte;

#define local_persist static
#define global_variable static
#define internal static

// NOTE: This macro is already defined in windows.h as RGB but i have
// to redefine it to match the wierd memory layout used in GDI
#define VSB_RGB(red, green, blue) (((uint8)(red))<<16)|(((uint8)(green))<<8)|((uint8)(blue))
#define VSB_RGBA(red, green, blue, alpha) (((uint8)(alpha))<<24) | VSB_RGB(red, green, blue)

#define KB(value) ((value)*1024LL)
#define MB(value) (KB(value)*1024LL)
#define GB(value) (MB(value)*1024LL)
#define TB(value) (GB(value)*1024LL)

#define ArrayCount(array) ((sizeof(array) / sizeof((array)[0]))
#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

#define Max(a, b) ((a)>(b)) ? a : b
#define Min(a, b) ((a)<(b)) ? a : b
#define Clamp(value, min, max) ((value)>(max)) ? (max) : (((value)<(min)) ? (min) : (value))

#define ABS(value) (((value)>=0) ? (value) : -(value))
