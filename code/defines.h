#pragma once

// TODO: maybe include stdint.h for these typedefs? Is it worth it?
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

#define VBS_WINDOW_WIDTH 1280
#define VBS_WINDOW_HEIGHT 720

// NOTE: This macro is already defined in windows.h as RGB but i have
// to redefine it to match the wierd memory layout used in GDI
#define VSB_RGB(red, green, blue) (((uint8)(red))<<16)|(((uint8)(green))<<8)|((uint8)(blue))