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
#include <guicolours.h>
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
static PTR framebuffer;

static GraphicsInfo arch_getGraphicsModeInfo();
static void draw_cursor (const KGraphicsArea* g);
#if ARCH == x86
static void arch_waitForNextVerticalRetrace();
#endif

#if CONFIG_GXMODE_FONT_WIDTH == 8
// RIGHT most bit in glyph is the value for LEFT most pixel of the glyph
U32 glyph_mask[] = { 1 << 7, 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 2, 1 << 1, 1 << 0 };
#endif

#if ARCH == x86
static GraphicsInfo arch_getGraphicsModeInfo()
{
    FUNC_ENTRY();
    BootGraphicsModeInfo gmi = kboot_getGraphicsModeInfo();

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

static void draw_cursor (const KGraphicsArea* g)
{
    UINT mouse_x = 600; // Some random location at this point
    UINT mouse_y = 400;
    graphics_rect (g, mouse_x, mouse_y, 10, 10, MOUSE_BG_COLOR);
    kgraphics_hline (g, mouse_x, mouse_y, 10, 2, MOUSE_FG_COLOR);
    kgraphics_vline (g, mouse_x, mouse_y, 10, 2, MOUSE_FG_COLOR);
}

void kgraphics_drawstring (const KGraphicsArea* g, UINT x, UINT y, const char* text, Color fg,
                           Color bg)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, text: %px, fg: %u, bg: %px", g, x, y, text, fg, bg);
    for (const char* ch = text; *ch != '\0'; ch++) {
        graphics_drawfont (g, x, y, (UCHAR)*ch, fg, bg);
        x += CONFIG_GXMODE_FONT_WIDTH;
    }
}

void graphics_drawfont (const KGraphicsArea* g, UINT x, UINT y, UCHAR a, Color fg, Color bg)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, char: %x, fg: %u, bg: %px", g, x, y, a, fg, bg);

    const U8* glyph  = gxi.fontsData + (a * BOOT_FONTS_GLYPH_BYTES);
    SIZE bytesPerRow = g->bytesPerRow;
    U8* start        = (U8*)g->surface + (y * bytesPerRow) + (x * g->bytesPerPixel);

    GxColor* fgColor = (GxColor*)&fg;
    GxColor* bgColor = (GxColor*)&bg;

    for (UINT y = 0; y < CONFIG_GXMODE_FONT_HEIGHT; y++, glyph++) {
        GxColor* row = (GxColor*)start;
        for (UINT x = 0; x < CONFIG_GXMODE_FONT_WIDTH; x++, row++) {
            *row = (*glyph & glyph_mask[x]) ? *fgColor : *bgColor;
        }
        start += bytesPerRow;
    }
}

void kgraphics_blit (const KGraphicsArea* destg, UINT x, UINT y, const KGraphicsArea* srcg)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, src area: %px", destg, x, y, srcg);

    k_assert (destg->bytesPerPixel == srcg->bytesPerPixel, "Graphics area mismatch");

    SIZE bytesPerPixel = destg->bytesPerPixel;
    SIZE bytesPerRow   = destg->bytesPerRow;
    U8* start          = (U8*)destg->surface + (y * bytesPerRow) + (x * bytesPerPixel);
    UINT h             = srcg->height_px;
    UINT w             = srcg->width_px;
    U8* bytes          = srcg->surface;

    for (; h > 0; h--) {
        GxColor* row = (GxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++, bytes += bytesPerPixel) {
            *row = *(GxColor*)bytes;
        }
        start += bytesPerRow;
    }
}

void graphics_image_raw (const KGraphicsArea* g, UINT x, UINT y, UINT w, UINT h, UINT bytesPerPixel,
                         U8* bytes)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, w: %u, h: %u, bytes: %px", g, x, y, w, h, bytes);

    SIZE bytesPerRow = g->bytesPerRow;
    U8* start        = (U8*)g->surface + (y * bytesPerRow) + (x * g->bytesPerPixel);

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

void graphics_putpixel (const KGraphicsArea* g, UINT x, UINT y, Color color)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, color: %x", g, x, y, color);

    GxColor* start = (GxColor*)(g->surface + (y * g->bytesPerRow) + (x * g->bytesPerPixel));
    GxColor* col   = (GxColor*)&color;
    *start         = *col;
}

void graphics_rect (const KGraphicsArea* g, UINT x, UINT y, UINT w, UINT h, Color color)
{
    FUNC_ENTRY ("area: %px, x: %u, y: %u, w: %u, h: %u, color: %x", g, x, y, w, h, color);

    SIZE bytesPerRow = g->bytesPerRow;
    U8* start        = (U8*)g->surface + (y * bytesPerRow) + (x * g->bytesPerPixel);

    GxColor* col = (GxColor*)&color;

    for (; h > 0; h--) {
        GxColor* row = (GxColor*)start;
        for (UINT lw = w; lw > 0; lw--, row++) {
            *row = *col;
        }
        start += bytesPerRow;
    }
}

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
                                     VMM_MEMMAP_FLAG_IMMCOMMIT | VMM_MEMMAP_FLAG_KERNEL_PAGE,
                                     NULL))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    kvmm_setAddressSpaceMetadata (g_kstate.context, (PTR)framebuffer, "vesafb", NULL);

    // Store a copy of basic/global graphics mode information.
    g_kstate.gx_back.bytesPerPixel    = gxi.bytesPerPixel;
    g_kstate.gx_back.bytesPerRow      = gxi.bytesPerScanLine;
    g_kstate.gx_back.width_px         = gxi.xResolution;
    g_kstate.gx_back.height_px        = gxi.yResolution;
    g_kstate.gx_back.surfaceSizeBytes = PAGEFRAMES_TO_BYTES (szPages);
    if (!(g_kstate.gx_back.surface = (U8*)
              kvmm_memmap (g_kstate.context, 0, NULL, szPages,
                           VMM_MEMMAP_FLAG_IMMCOMMIT | VMM_MEMMAP_FLAG_KERNEL_PAGE, NULL))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    kvmm_setAddressSpaceMetadata (g_kstate.context, (PTR)g_kstate.gx_back.surface, "gxbackbuffer",
                                  NULL);

    KERNEL_PHASE_SET (KERNEL_PHASE_STATE_GRAPHICS_READY);
    return true;
}

void kgraphis_flush()
{
    if (g_kstate.phase < KERNEL_PHASE_STATE_GRAPHICS_READY) {
        return; // Graphics mode is not ready
    }

    KGraphicsArea* backbuffer = (KGraphicsArea*)&g_kstate.gx_back;

    // Backbuffer must exactly match the vesa framebuffer
    k_assert (backbuffer != NULL && (void*)framebuffer != NULL, "Graphics buffers cannot be NULL");
    k_assert (g_kstate.gx_back.bytesPerRow == gxi.bytesPerScanLine, "Invalid backbuffer");
    k_assert (g_kstate.gx_back.bytesPerPixel == gxi.bytesPerPixel, "Invalid backbuffer");
    k_assert (g_kstate.gx_back.width_px == gxi.xResolution, "Invalid backbuffer");
    k_assert (g_kstate.gx_back.height_px == gxi.yResolution, "Invalid backbuffer");

    arch_waitForNextVerticalRetrace();
    draw_cursor (backbuffer);

    k_memcpy ((void*)framebuffer, backbuffer->surface, backbuffer->surfaceSizeBytes);
}
