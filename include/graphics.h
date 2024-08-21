/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel graphics header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

typedef U32 KernelGxColor;

typedef struct RGBColor {
        U8 blue;
        U8 green;
        U8 red;
} __attribute__ ((packed)) RGBColor;

typedef U8 IndexColor8Bit;

void graphics_rect (UINT x, UINT y, UINT h, UINT w, KernelGxColor color);
void graphics_image (UINT x, UINT y, UINT w, UINT h, U8* bytes);
void graphics_drawfont (UINT x, UINT y, UCHAR a, KernelGxColor fg, KernelGxColor bg);

bool graphics_init();
