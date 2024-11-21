/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 -  App library - Graphics
 * --------------------------------------------------------------------------------------------------
 */

#include <kstdlib.h>
#include <types.h>
#include <cm/graphics.h>
#include <cm/app.h>

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

#if GRAPHICS_BPP == 8
typedef IndexColor8Bit GxColor;
#elif GRAPHICS_BPP == 24
typedef RGBColor24Bits GxColor;
#elif GRAPHICS_BPP == 32
typedef RGBColor32Bits GxColor;
#endif

void graphics_blit (const GraphicsContext* destg, UINT x, UINT y, const GraphicsContext* srcg)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, src area: %px", destg, x, y, srcg);

    // k_assert (destg->bytesPerPixel == srcg->bytesPerPixel, "Graphics area mismatch");

    SIZE bytesPerPixel = destg->bytesPerPixel;
    SIZE bytesPerRow   = destg->bytesPerRow;
    U8* start          = (U8*)destg->buffer + (y * bytesPerRow) + (x * bytesPerPixel);
    UINT h             = srcg->height_px;
    UINT w             = srcg->width_px;
    U8* bytes          = srcg->buffer;

    for (; h > 0; h--) {
        GxColor* row = (GxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++, bytes += bytesPerPixel) {
            *row = *(GxColor*)bytes;
        }
        start += bytesPerRow;
    }
}

void graphics_image_ppm (const GraphicsContext* g, UINT x, UINT y, UINT w, UINT h,
                         UINT bytesPerPixel, U8* bytes)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, w: %u, h: %u, bytes: %px", g, x, y, w, h, bytes);

    SIZE bytesPerRow = g->bytesPerRow;
    U8* start        = (U8*)g->buffer + (y * bytesPerRow) + (x * g->bytesPerPixel);

    for (; h > 0; h--) {
        GxColor* row = (GxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++, bytes += bytesPerPixel) {
#if CONFIG_GXMODE_BITSPERPIXEL == 32 || CONFIG_GXMODE_BITSPERPIXEL == 24
    #if CONFIG_GXMODE_BITSPERPIXEL == 32
            row->alpha = 0;
    #endif
            row->red   = *bytes;
            row->green = *(bytes + 1);
            row->blue  = *(bytes + 2);
#elif CONFIG_GXMODE_BITSPERPIXEL == 8
            *row = *bytes;
#endif
        }
        start += bytesPerRow;
    }
}

void graphics_putpixel (const GraphicsContext* g, UINT x, UINT y, Color color)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, color: %x", g, x, y, color);

    GxColor* start = (GxColor*)(g->buffer + (y * g->bytesPerRow) + (x * g->bytesPerPixel));
    GxColor* col   = (GxColor*)&color;
    *start         = *col;
}

void graphics_rect (const GraphicsContext* g, UINT x, UINT y, UINT w, UINT h, Color color)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, w: %u, h: %u, color: %x", g, x, y, w, h, color);

    SIZE bytesPerRow = g->bytesPerRow;
    U8* start        = (U8*)g->buffer + (y * bytesPerRow) + (x * g->bytesPerPixel);

    GxColor* col = (GxColor*)&color;

    for (; h > 0; h--) {
        GxColor* row = (GxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++) {
            *row = *col;
        }
        start += bytesPerRow;
    }
}
