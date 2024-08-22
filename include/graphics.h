/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel graphics header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

typedef U32 KernelGxColor;

typedef struct Vec4 {
    U8 a;
    U8 b;
    U8 c;
    U8 d;
} __attribute__ ((packed)) Vec4;

typedef U8 IndexColor8Bit;
typedef struct RGBColor24Bits {
    U8 blue;
    U8 green;
    U8 red;
} __attribute__ ((packed)) RGBColor24Bits;

typedef struct RGBColor32Bits {
    U8 blue;
    U8 green;
    U8 red;
    U8 alpha;
} __attribute__ ((packed)) RGBColor32Bits;

#if CONFIG_GXMODE_BITSPERPIXEL == 8
typedef IndexColor8Bit Color;
#elif CONFIG_GXMODE_BITSPERPIXEL == 24
typedef RGBColor24Bits Color;
#elif CONFIG_GXMODE_BITSPERPIXEL == 32
typedef RGBColor32Bits Color;
#endif

void graphics_rect (UINT x, UINT y, UINT h, UINT w, KernelGxColor color);
void graphics_image_raw (UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel, U8* bytes);
void graphics_drawfont (UINT x, UINT y, UCHAR a, KernelGxColor fg, KernelGxColor bg);

bool graphics_init();
