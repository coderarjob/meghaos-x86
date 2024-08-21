/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel graphics
 *
 * Graphics operations for the Kernel
 * ---------------------------------------------------------------------------
 */

#include <types.h>
#include <config.h>
#include <disp.h>
#include <kernel.h>
#include <kdebug.h>
#include <kerror.h>
#include <kstdlib.h>
#include <graphics.h>
#if ARCH == x86
    #include <x86/boot.h>
#endif

typedef struct GraphicsInfo {
    U16 xResolution;
    U16 yResolution;
    U8 bytesPerPixel;
    Physical framebufferPhysicalPtr;
    U16 bytesPerScanLine;
    U8* fontsData;
} __attribute__ ((packed)) GraphicsInfo;

static GraphicsInfo gxi;
static GraphicsInfo arch_getGraphicsModeInfo();

#if CONFIG_GXMODE_BITSPERPIXEL == 8
void graphics_drawfont (UINT x, UINT y, UCHAR a, KernelGxColor fg, KernelGxColor bg)
{
    FUNC_ENTRY ("x: %u, y: %u, char: %x, fg: %u, bg: %px", x, y, a, fg, bg);

    const U8* glyph    = gxi.fontsData + (a * BOOT_FONTS_GLYPH_BYTES);
    SIZE bytesPerPixel = gxi.bytesPerPixel;
    PTR start          = g_kstate.framebuffer + (y * gxi.bytesPerScanLine) + (x * bytesPerPixel);

    IndexColor8Bit fgColor = (IndexColor8Bit)fg;
    IndexColor8Bit bgColor = (IndexColor8Bit)bg;

    for (UINT y = 0; y < CONFIG_GXMODE_FONT_HEIGHT; y++, glyph++) {
        PTR row = start;
        for (UINT x = 0; x < CONFIG_GXMODE_FONT_WIDTH; x++, row += bytesPerPixel) {
            *(U8*)row = (*glyph & (1 << (CONFIG_GXMODE_FONT_WIDTH - 1 - x))) ? fgColor : bgColor;
        }
        start += gxi.bytesPerScanLine;
    }
}

void graphics_image_raw (UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel, U8* bytes)
{
    FUNC_ENTRY ("x: %u, y: %u, w: %u, h: %u, bytes: %px", x, y, w, h, bytes);

    k_assert (bytesPerPixel == 1, "Only 8 bit color image is supported");

    PTR start = g_kstate.framebuffer + (y * gxi.bytesPerScanLine) + (x * gxi.bytesPerPixel);

    for (; h > 0; h--) {
        PTR row = start;
        for (UINT lw = w; lw > 0; lw--, bytes+=bytesPerPixel {
            *(U8*)row = *bytes;
            row += bytesPerPixel;
        }
        start += gxi.bytesPerScanLine;
    }
}

void graphics_rect (UINT x, UINT y, UINT w, UINT h, U32 color)
{
    FUNC_ENTRY ("x: %u, y: %u, w: %u, h: %u, color: %x", x, y, w, h, color);

    SIZE bytesPerPixel = gxi.bytesPerPixel;
    U8* start = (U8*)g_kstate.framebuffer + (y * gxi.bytesPerScanLine) + (x * bytesPerPixel);

    IndexColor8Bit col = (IndexColor8Bit)color;

    for (; h > 0; h--) {
        U8* row = start;
        for (UINT lw = w; lw > 0; lw--) {
            *row = col;
            row += bytesPerPixel;
        }
        start += gxi.bytesPerScanLine;
    }
}
#endif // CONFIG_GXMODE_BITSPERPIXEL == 8

#if CONFIG_GXMODE_BITSPERPIXEL == 32
void graphics_drawfont (UINT x, UINT y, UCHAR a, KernelGxColor fg, KernelGxColor bg)
{
    FUNC_ENTRY ("x: %u, y: %u, char: %x, fg: %u, bg: %px", x, y, a, fg, bg);

    const U8* glyph    = gxi.fontsData + (a * BOOT_FONTS_GLYPH_BYTES);
    SIZE bytesPerPixel = gxi.bytesPerPixel;
    U8* start = (U8*)g_kstate.framebuffer + (y * gxi.bytesPerScanLine) + (x * bytesPerPixel);

    for (UINT y = 0; y < CONFIG_GXMODE_FONT_HEIGHT; y++, glyph++) {
        U32* row = (U32*)start;
        for (UINT x = 0; x < CONFIG_GXMODE_FONT_WIDTH; x++, row++) {
            *row = (*glyph & (1 << (CONFIG_GXMODE_FONT_WIDTH - 1 - x))) ? fg : bg;
        }
        start += gxi.bytesPerScanLine;
    }
}

void graphics_image_raw (UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel, U8* bytes)
{
    FUNC_ENTRY ("x: %u, y: %u, w: %u, h: %u, bytes: %px", x, y, w, h, bytes);

    U8* start = (U8*)g_kstate.framebuffer + (y * gxi.bytesPerScanLine) + (x * gxi.bytesPerPixel);

    for (; h > 0; h--) {
        Vec4* row = (Vec4*)start;
        for (UINT lw = w; lw > 0; lw--, row++, bytes += bytesPerPixel) {
            row->d = 0;            // Image does not have alpha value.
            row->c = *bytes;       // Red
            row->b = *(bytes + 1); // Green
            row->a = *(bytes + 2); // Blue
        }
        start += gxi.bytesPerScanLine;
    }
}

void graphics_rect (UINT x, UINT y, UINT w, UINT h, KernelGxColor color)
{
    FUNC_ENTRY ("x: %u, y: %u, w: %u, h: %u, color: %x", x, y, w, h, color);

    SIZE bytesPerPixel = gxi.bytesPerPixel;
    U8* start = (U8*)g_kstate.framebuffer + (y * gxi.bytesPerScanLine) + (x * bytesPerPixel);

    for (; h > 0; h--) {
        KernelGxColor* row = (KernelGxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++) {
            *row = color;
        }
        start += gxi.bytesPerScanLine;
    }
}
#endif

#if ARCH == x86
static GraphicsInfo arch_getGraphicsModeInfo()
{
    FUNC_ENTRY();
    GraphisModeInfo gmi = kboot_getGraphicsModeInfo();

    INFO ("Mode: %x", gmi.graphicsMode);
    INFO ("VBE Version: %x", gmi.vbeVersion);
    INFO ("FrameBuffer: %x", gmi.framebufferPhysicalPtr);
    INFO ("Resolution: %u x %u, %ubpp", gmi.xResolution, gmi.yResolution, gmi.bitsPerPixel);
    INFO ("BytesPerScanLine: %x", gmi.bytesPerScanLine);

    //  Store required infromation in arch independent structure.
    GraphicsInfo gxi = { .bytesPerPixel          = gmi.bitsPerPixel / 8,
                         .bytesPerScanLine       = gmi.bytesPerScanLine,
                         .framebufferPhysicalPtr = gmi.framebufferPhysicalPtr,
                         .xResolution            = gmi.xResolution,
                         .yResolution            = gmi.yResolution,
                         .fontsData              = (U8*)kboot_getFontData() };

    return gxi;
}
#endif

bool graphics_init()
{
    FUNC_ENTRY();

    gxi           = arch_getGraphicsModeInfo();
    SIZE szBytes  = (SIZE)gxi.bytesPerScanLine * gxi.yResolution;
    SIZE szPages  = BYTES_TO_PAGEFRAMES_CEILING (szBytes);
    Physical fbpa = gxi.framebufferPhysicalPtr;
    if (fbpa.val) {
        if (!(g_kstate.framebuffer = kvmm_memmap (g_kstate.context, (PTR)NULL, &fbpa, szPages,
                                                  VMM_MEMMAP_FLAG_IMMCOMMIT, NULL))) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false);
        }
        return true;
    }
    return false;
}
