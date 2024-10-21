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
#include <memmanage.h>
#if ARCH == x86
    #include <x86/boot.h>
    #include <x86/io.h>
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
static PTR framebuffer;
static PTR backbuffer;
static SIZE framebuffer_size_bytes;
#if ARCH == x86
static void arch_waitForNextVerticalRetrace();
#endif

#if CONFIG_GXMODE_FONT_WIDTH == 8
// RIGHT most bit in glyph is the value for LEFT most pixel of the glyph
U32 glyph_mask[] = { 1 << 7, 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 2, 1 << 1, 1 << 0 };
#endif

void graphics_drawfont (UINT x, UINT y, UCHAR a, Color fg, Color bg)
{
    FUNC_ENTRY ("x: %u, y: %u, char: %x, fg: %u, bg: %px", x, y, a, fg, bg);

    const U8* glyph = gxi.fontsData + (a * BOOT_FONTS_GLYPH_BYTES);
    U8* start       = (U8*)backbuffer + (y * gxi.bytesPerScanLine) + (x * gxi.bytesPerPixel);

    GxColor* fgColor = (GxColor*)&fg;
    GxColor* bgColor = (GxColor*)&bg;

    for (UINT y = 0; y < CONFIG_GXMODE_FONT_HEIGHT; y++, glyph++) {
        GxColor* row = (GxColor*)start;
        for (UINT x = 0; x < CONFIG_GXMODE_FONT_WIDTH; x++, row++) {
            *row = (*glyph & glyph_mask[x]) ? *fgColor : *bgColor;
        }
        start += gxi.bytesPerScanLine;
    }
}

void graphics_image_raw (UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel, U8* bytes)
{
    FUNC_ENTRY ("x: %u, y: %u, w: %u, h: %u, bytes: %px", x, y, w, h, bytes);

    U8* start = (U8*)backbuffer + (y * gxi.bytesPerScanLine) + (x * gxi.bytesPerPixel);

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
        start += gxi.bytesPerScanLine;
    }
}

void graphics_putpixel (UINT x, UINT y, Color color)
{
    FUNC_ENTRY ("x: %u, y: %u, color: %x", x, y, color);

    GxColor* start = (GxColor*)(backbuffer + (y * gxi.bytesPerScanLine) + (x * gxi.bytesPerPixel));
    GxColor* col   = (GxColor*)&color;
    *start         = *col;
}

void graphics_rect (UINT x, UINT y, UINT w, UINT h, Color color)
{
    FUNC_ENTRY ("x: %u, y: %u, w: %u, h: %u, color: %x", x, y, w, h, color);

    SIZE bytesPerPixel = gxi.bytesPerPixel;
    U8* start          = (U8*)backbuffer + (y * gxi.bytesPerScanLine) + (x * bytesPerPixel);

    GxColor* col = (GxColor*)&color;

    for (; h > 0; h--) {
        GxColor* row = (GxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++) {
            *row = *col;
        }
        start += gxi.bytesPerScanLine;
    }
}

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

static void arch_waitForNextVerticalRetrace()
{
    // Wait for the ongoing Vertical Retrace to end.
    while (ioread (0x3DA) & 0x8)
        ;

    // Wait for next Vertical Retrace to start.
    while (!(ioread (0x3DA) & 0x8))
        ;
}
#endif

bool graphics_init()
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_VMM_READY);

    gxi           = arch_getGraphicsModeInfo();
    SIZE szPages  = BYTES_TO_PAGEFRAMES_CEILING ((SIZE)gxi.bytesPerScanLine * gxi.yResolution);
    Physical fbpa = gxi.framebufferPhysicalPtr;
    if (!fbpa.val) {
        RETURN_ERROR (ERR_DEVICE_NOT_READY, false);
    }

    if (!(framebuffer = kvmm_memmap (g_kstate.context, (PTR)NULL, &fbpa, szPages,
                                     VMM_MEMMAP_FLAG_IMMCOMMIT, NULL))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    if (!(backbuffer = kvmm_memmap (g_kstate.context, 0, NULL, szPages, VMM_MEMMAP_FLAG_NONE,
                                    NULL))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    framebuffer_size_bytes = PAGEFRAMES_TO_BYTES (szPages);

    KERNEL_PHASE_SET (KERNEL_PHASE_STATE_GRAPHICS_READY);
    return true;
}

void kgraphis_flush()
{
    if (g_kstate.phase < KERNEL_PHASE_STATE_GRAPHICS_READY) {
        return; // Graphics mode is not ready
    }

    k_assert ((void*)backbuffer != NULL && (void*)framebuffer != NULL,
              "Graphics buffers cannot be NULL");

    arch_waitForNextVerticalRetrace();

    k_memcpy ((void*)framebuffer, (void*)backbuffer, framebuffer_size_bytes);
}
