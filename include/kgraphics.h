/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel graphics header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#ifndef GRAPHICS_MODE_ENABLED
    #error "Gui demo only works in graphics mode"
#endif

#include <types.h>

// This is a arch independent color type large enough for all the color modes supported.
typedef U32 Color;

typedef U8 IndexColor8Bit;

// RGB is BGR in memory and that is the order in which the fields apprear. This simplifies when
// assignning/copying color values from memory (like in graphics_rect, graphics_drawfont etc).
// Note: PPM and RAW image formats etc, which store RGB however store color in R,G,B and not B,G,R.
typedef struct RGBColor24Bits {
    U8 blue;
    U8 green;
    U8 red;
} __attribute__ ((packed)) RGBColor24Bits;

// ARGB is BGRA in memory and that is the order in which the fields apprear. This simplifies when
// assignning/copying color values from memory (like in graphics_rect, graphics_drawfont etc).
// Note: PPM and RAW image formats etc, which store RGB however store color in R,G,B and not B,G,R.
typedef struct RGBColor32Bits {
    U8 blue;
    U8 green;
    U8 red;
    U8 alpha;
} __attribute__ ((packed)) RGBColor32Bits;

typedef struct KGraphicsArea {
    UINT width_px;
    UINT height_px;
    UINT bytesPerPixel;
    UINT bytesPerRow;
    U8* buffer;
    SIZE bufferSizeBytes;
} KGraphicsArea;

#if CONFIG_GXMODE_BITSPERPIXEL == 8
typedef IndexColor8Bit GxColor;
#elif CONFIG_GXMODE_BITSPERPIXEL == 24
typedef RGBColor24Bits GxColor;
#elif CONFIG_GXMODE_BITSPERPIXEL == 32
typedef RGBColor32Bits GxColor;
#endif

void kgraphics_rect (const KGraphicsArea* g, UINT x, UINT y, UINT w, UINT h, Color color);
void kgraphics_image_raw (const KGraphicsArea* g, UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel,
                         U8* bytes);
void kgraphics_drawfont (const KGraphicsArea* g, UINT x, UINT y, UCHAR a, Color fg, Color bg);
void kgraphics_putpixel (const KGraphicsArea* g, UINT x, UINT y, Color color);
void kgraphis_flush(void);
void kgraphics_blit (const KGraphicsArea* destg, UINT x, UINT y, const KGraphicsArea* srcg);
bool kgraphics_init(void);
void kgraphics_drawstring (const KGraphicsArea* g, UINT x, UINT y, const char* text, Color fg,
                           Color bg);

static inline void kgraphics_hline (const KGraphicsArea* g, UINT x, UINT y, UINT w, UINT th,
                                    Color color)
{
    kgraphics_rect (g, x, y, w, th, color);
}

static inline void kgraphics_vline (const KGraphicsArea* g, UINT x, UINT y, UINT h, UINT th,
                                    Color color)
{
    kgraphics_rect (g, x, y, th, h, color);
}

/* Draws rectangle with borders on the inside of a rectangle */
static inline void kgraphics_inborder (const KGraphicsArea* g, UINT x, UINT y, UINT w, UINT h,
                                       UINT th, Color color)
{
    kgraphics_hline (g, x, y, w, th, color);            // Top
    kgraphics_vline (g, x, y, h, th, color);            // Left
    kgraphics_vline (g, (x + w - th), y, h, th, color); // Right
    kgraphics_hline (g, x, (y + h - th), w, th, color); // Bottom
}
