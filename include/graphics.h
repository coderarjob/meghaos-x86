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

void graphics_rect (UINT x, UINT y, UINT h, UINT w, KernelGxColor color);
void graphics_image_raw (UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel, U8* bytes);
void graphics_drawfont (UINT x, UINT y, UCHAR a, KernelGxColor fg, KernelGxColor bg);

bool graphics_init();
