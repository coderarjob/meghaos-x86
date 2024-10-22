/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Window compositor
 * ---------------------------------------------------------------------------
 */

#include <types.h>
#include <common/bitmap.h>
#include <graphics.h>
#include <config.h>
#include <kernel.h>
#include <compositor.h>
#include <kdebug.h>
#include <panic.h>

static BitmapState bitmap[(WINDOW_MANAGER_GRID_CELL_COUNT / 8) + 1];
static Bitmap windowgrid;

static bool isValid (UINT windowcell, BitmapState old, BitmapState new)
{
    (void)windowcell;
    if (!(old ^ new)) {
        k_panic ("Window cell already occupied/free");
    }
    return true;
}

void kcompose_init()
{
    Bitmap b   = { bitmap, sizeof (bitmap), 1, isValid };
    windowgrid = b;
}

void kcompose_flush()
{
    const UINT height = WINDOW_MANAGER_GRID_HEIGHT (g_kstate.gx);
    const UINT width  = WINDOW_MANAGER_GRID_WIDTH (g_kstate.gx);

    U32 colors[WINDOW_MANAGER_GRID_CELL_COUNT] = { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00 };

    INFO ("Window manager: Size = (%u, %u)", width, height);

    for (UINT c = 0; c < WINDOW_MANAGER_GRID_COLS_MAX; c++) {
        for (UINT r = 0; r < WINDOW_MANAGER_GRID_ROWS_MAX; r++) {
            UINT top  = WINDOW_MANAGER_GRID_TOP (g_kstate.gx, r);
            UINT left = WINDOW_MANAGER_GRID_LEFT (g_kstate.gx, c);
            U32 color = colors[r * WINDOW_MANAGER_GRID_COLS_MAX + c];

            INFO ("Window manager: Position = (%u, %u)", left, top);
            graphics_rect (left, top, width, height, color);
            k_delay(1000);
        }
    }
}
