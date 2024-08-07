#pragma once

#include <stdbool.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

typedef _Bool b8;
typedef int b32;

typedef struct color {
  f32 r, g, b, a;
} color;

typedef struct extent2d {
  f32 x, y, w, h;
} extent2d;

#define INVALID_ID 4294967295U

// Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size.
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define TRUE 1
#define FALSE 0
#define SF_NULL (void *)0

#define CLAMP(value, min, max)                                                 \
  (value <= min) ? min : (value >= max) ? max : value

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define SPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define SPLATFORM_LINUX 1
#if defined(__ANDROID__)
#define SPLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define SPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define SPLATFORM_POSIX 1
#endif

#ifdef SAPEXPORT
#ifdef _MSC_VER
#define SAPI __declspec(dllexport)
#else
#define SAPI __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define SAPI __declspec(dllimport)
#else
#define SAPI
#endif
#endif

#ifdef _MSC_VER
#define INLINE __forceinline
#define NOINLINE __declspec(noinline)
#else
#define INLINE static inline
#define KNOINLINE
#endif

#ifdef _MSC_VER
#define ALIGN(x) __declspec(align(x))
#else
#define ALIGN(x) __attribute__((aligned(x)))
#endif
