/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - App library - Potable Types
 * ---------------------------------------------------------------------------
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef int32_t S32;
typedef uint64_t U64;
typedef int64_t S64;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef size_t SIZE;

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
/* For any environment, UINT and INT will match the native width of registers. So when fast speed is
 * required, or variables are platform independent, use UINT or INT instead of U32 or S32.*/

typedef uintptr_t PTR;
typedef S64 LLONG;
typedef U64 ULLONG;
typedef S32 INT;
typedef S32 LONG;
typedef U32 UINT;
typedef U32 ULONG;
#endif

typedef INT Handle;
