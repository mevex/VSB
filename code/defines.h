// NOTE: This is a collection of macros and typedefs widely used
// throughout the project

#pragma once
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef float f32;
typedef double f64;

typedef ui8 byte;

#define local_persist static
#define global_variable static
#define internal static

// NOTE(mevex): Constants
#define ZERO 0.001f

// NOTE: This macro is already defined in windows.h as RGB but i have
// to redefine it to match the wierd memory layout used in GDI
#define VSB_RGBA(red, green, blue, alpha) ui32(((ui8(alpha))<<24)|((ui8(red))<<16)|((ui8(green))<<8)|(ui8(blue)))
#define VSB_RGB(red, green, blue) VSB_RGBA(red, green, blue, 0xff)

#define ExtractA(color) (((color) >> 24) & 0xff)
#define ExtractR(color) (((color) >> 16) & 0xff)
#define ExtractG(color) (((color) >> 8)  & 0xff)
#define ExtractB(color) (((color) >> 0)  & 0xff)

#define KB(value) ((value)*1024LL)
#define MB(value) (KB(value)*1024LL)
#define GB(value) (MB(value)*1024LL)
#define TB(value) (GB(value)*1024LL)

#define ArrayCount(array) ((sizeof(array) / sizeof((array)[0]))

#include <assert.h>
#define Assert assert
