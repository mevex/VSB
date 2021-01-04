#pragma once

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef float f32;
typedef double f64;

#define local_persist static
#define global_variable static
#define internal static

// NOTE: This macro is already defined in windows.h as RGB but i have
// to redefine it to match the wierd memory layout used in GDI
#define VSB_RGB(red, green, blue) (((uint8)(red))<<16)|(((uint8)(green))<<8)|((uint8)(blue))

#define KB(value) ((value)*1024LL)
#define MB(value) (KB(value)*1024LL)
#define GB(value) (MB(value)*1024LL)
#define TB(value) (GB(value)*1024LL)

#define ArrayCount(array) ((sizeof(array) / sizeof((array)[0]))
#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

#define Max(a, b) ((a)>(b)) ? a : b
#define Min(a, b) ((a)<(b)) ? a : b
#define Clamp(value, min, max) ((value)>(max)) ? (max) : (((value)<(min)) ? (min) : (value))
