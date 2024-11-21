/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel graphics header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#ifdef KERNEL
    #include <cm/osif.h>
#else
    #include <osif.h>
#endif

// This is a arch independent color type large enough for all the color modes supported.
typedef U32 Color;
typedef OSIF_WindowFrameBufferInfo GraphicsContext;

void graphics_rect (const GraphicsContext* g, UINT x, UINT y, UINT w, UINT h, Color color);
void graphics_image_ppm (const GraphicsContext* g, UINT x, UINT y, UINT w, UINT h,
                         UINT bytesPerPixel, U8* bytes);
void graphics_drawfont (const GraphicsContext* g, UINT x, UINT y, UCHAR a, Color fg, Color bg);
void graphics_putpixel (const GraphicsContext* g, UINT x, UINT y, Color color);
void kgraphics_blit (const GraphicsContext* destg, UINT x, UINT y, const GraphicsContext* srcg);

static inline void kgraphics_hline (const GraphicsContext* g, UINT x, UINT y, UINT w, UINT th,
                                    Color color)
{
    graphics_rect (g, x, y, w, th, color);
}

static inline void kgraphics_vline (const GraphicsContext* g, UINT x, UINT y, UINT h, UINT th,
                                    Color color)
{
    graphics_rect (g, x, y, th, h, color);
}

/* Draws rectangle with borders on the inside of a rectangle */
static inline void kgraphics_inborder (const GraphicsContext* g, UINT x, UINT y, UINT w, UINT h,
                                       UINT th, Color color)
{
    kgraphics_hline (g, x, y, w, th, color);            // Top
    kgraphics_vline (g, x, y, h, th, color);            // Left
    kgraphics_vline (g, (x + w - th), y, h, th, color); // Right
    kgraphics_hline (g, x, (y + h - th), w, th, color); // Bottom
}
