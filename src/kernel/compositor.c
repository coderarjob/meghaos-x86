/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Window compositor
 * ---------------------------------------------------------------------------
 */

#include <stdbool.h>
#include <types.h>
#include <kgraphics.h>
#include <config.h>
#include <kernel.h>
#include <compositor.h>
#include <kdebug.h>
#include <panic.h>
#include <kstdlib.h>
#include <kerror.h>
#include <intrusive_list.h>
#include <memmanage.h>
#include <process.h>
#include <guicolours.h>
#include <vmm.h>

#define WINDOW_BORDER_WIDTH_PX        (3U)
#define WINDOW_TITLE_BAR_TOP_PX       (0)
#define WINDOW_TITLE_BAR_LEFT_PX      (0)
#define WINDOW_TITLE_BAR_HEIGHT_PX    (20U)
#define WINDOW_TITLE_BAR_WIDTH_PX(wa) (wa->width_px)
#define WINDOW_TITLE_TOP_PX           (4)
#define WINDOW_TITLE_LEFT_PX          (WINDOW_BORDER_WIDTH_PX + 4)

#define WINDOW_WORKING_AREA_TOP_PX    (WINDOW_TITLE_BAR_HEIGHT_PX)
#define WINDOW_WORKING_AREA_LEFT_PX   (WINDOW_BORDER_WIDTH_PX)
#define WINDOW_WORKING_AREA_HEIGHT_PX(wa) \
    (wa->height_px - (WINDOW_TITLE_BAR_HEIGHT_PX + WINDOW_BORDER_WIDTH_PX))
#define WINDOW_WORKING_AREA_WIDTH_PX(wa) (wa->width_px - (2 * WINDOW_BORDER_WIDTH_PX))

static SIZE getWindowAreaSizeBytes(void)
{
    return WINMAN_GRID_CELL_WIDTH_PX() * WINMAN_GRID_CELL_HEIGHT_PX() *
           g_kstate.gx_backfb.bytesPerPixel;
}

static ListNode windowsListHead;
static UINT window_count;

static void destory_window (Window* win);

static void drawWindowDecorations (const KGraphicsArea* wa, const char* title)
{
    FUNC_ENTRY ("area: %px, title: %px", wa, title);

    UINT x, y, w, h;

    // ===============================
    // Title bar
    // ===============================
    x = WINDOW_TITLE_BAR_LEFT_PX;
    y = WINDOW_TITLE_BAR_TOP_PX;
    w = WINDOW_TITLE_BAR_WIDTH_PX (wa);
    h = WINDOW_TITLE_BAR_HEIGHT_PX;
    // Title bar background
    kgraphics_rect (wa, x, y, w, h, WINDOW_TITLE_BAR_BG_COLOR);

    // Title bar title string
    kgraphics_drawstring (wa, WINDOW_TITLE_LEFT_PX, WINDOW_TITLE_TOP_PX, title,
                          WINDOW_TITLE_BAR_FG_COLOR, WINDOW_TITLE_BAR_BG_COLOR);

    // Title bar bottom border
    y = WINDOW_TITLE_BAR_HEIGHT_PX;
    kgraphics_hline (wa, x - 1, y - 1, w, 1, COLOR_WHITE);

    // ===============================
    // 3D Border
    // ===============================
    x = WINDOW_TITLE_BAR_LEFT_PX;
    y = WINDOW_TITLE_BAR_TOP_PX;
    w = wa->width_px;
    h = wa->height_px;
    kgraphics_inborder (wa, x, y, w, h, WINDOW_BORDER_WIDTH_PX, WINDOW_BORDER_COLOR1);
    kgraphics_inborder (wa, x + 1, y + 1, w - 2, h - 2, 1, WINDOW_BORDER_COLOR2);

    // ===============================
    // Active area
    // ===============================
    x = WINDOW_WORKING_AREA_LEFT_PX;
    y = WINDOW_WORKING_AREA_TOP_PX;
    w = WINDOW_WORKING_AREA_WIDTH_PX (wa);
    h = WINDOW_WORKING_AREA_HEIGHT_PX (wa);
    kgraphics_rect (wa, x, y, w, h, WINDOW_BG_COLOR);
}

static Window* allocWindow (UINT processID, UINT screen_x, UINT screen_y)
{
    // Create new window graphics area
    KGraphicsArea windowArea = { .bytesPerPixel = g_kstate.gx_backfb.bytesPerPixel,
                                 .width_px      = WINMAN_GRID_CELL_WIDTH_PX(),
                                 .height_px     = WINMAN_GRID_CELL_HEIGHT_PX(),
                                 .bytesPerRow   = WINMAN_GRID_CELL_WIDTH_PX() *
                                                g_kstate.gx_backfb.bytesPerPixel };

    VMemoryManager* vmm        = kprocess_getCurrentContext();
    SIZE bufferSzPages         = BYTES_TO_PAGEFRAMES_CEILING (getWindowAreaSizeBytes());
    windowArea.bufferSizeBytes = PAGEFRAMES_TO_BYTES (bufferSzPages);
    if (!(windowArea.buffer = (U8*)kvmm_memmap (vmm, 0, NULL, bufferSzPages, VMM_MEMMAP_FLAG_NONE,
                                                NULL))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }
    kvmm_setAddressSpaceMetadata (vmm, (PTR)windowArea.buffer, "winfb", &processID);

    // Working area is derived from the window area. Its the rectangle in the middle that not
    // effected by the window decorations.
    KGraphicsArea workingArea = windowArea;
    workingArea.height_px     = WINDOW_WORKING_AREA_HEIGHT_PX ((&windowArea));
    workingArea.width_px      = WINDOW_WORKING_AREA_WIDTH_PX ((&windowArea));
    workingArea.buffer += WINDOW_WORKING_AREA_TOP_PX * workingArea.bytesPerRow +
                          WINDOW_WORKING_AREA_LEFT_PX * workingArea.bytesPerPixel;
    workingArea.bufferSizeBytes = workingArea.height_px * workingArea.bytesPerRow;

    // Create new associated window. The graphics areas will be part of and owned by this window.
    Window* newwin = NULL;
    if (!(newwin = kmalloc (sizeof (Window)))) {
        destory_window (newwin);
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    newwin->processID         = processID;
    newwin->position.screen_x = screen_x;
    newwin->position.screen_y = screen_y;
    newwin->windowArea        = windowArea;
    newwin->workingArea       = workingArea;
    list_init (&newwin->windowListNode);

    return newwin;
}

static void destory_window (Window* win)
{
    FUNC_ENTRY ("Window: %px", win);

    VMemoryManager* vmm = kprocess_getCurrentContext();
    if (!kvmm_free (vmm, (PTR)win->windowArea.buffer)) {
        BUG(); // Should be able to free.
    }

    if (!kfree (win)) {
        BUG(); // Should be able to free.
    }
}

void kcompose_init(void)
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_GRAPHICS_READY);

    list_init (&windowsListHead);
    window_count = 0;
}

bool kcompose_destroyWindow (Window* win)
{
    FUNC_ENTRY ("Window: %px", win);

    // Remove window from the windows list
    list_remove (&win->windowListNode);

    // Free the memories related to the window
    destory_window (win);
    return true;
}

Window* kcompose_createWindow (const char* const title)
{
    FUNC_ENTRY ("Title: %px", title);

    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_KERNEL_READY);

    // Find free window cell
    if (window_count == WINMAN_GRID_CELL_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, NULL);
    }

    UINT col = WINMAN_GRID_CELL_COL (window_count);
    UINT row = WINMAN_GRID_CELL_ROW (window_count);
    window_count++;

    // Create new window and paint area
    Window* newwin = allocWindow (kprocess_getCurrentPID(), WINMAN_GRID_CELL_X (col),
                                  WINMAN_GRID_CELL_Y (row));
    if (!newwin) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    // Add window to the windows list
    list_add_after (&windowsListHead, &newwin->windowListNode);

    // Draw window decorations
    drawWindowDecorations (&newwin->windowArea, title);

    INFO ("Window created: Position = %u, %u, Size: %u, %u", newwin->position.screen_x,
          newwin->position.screen_y, newwin->windowArea.width_px, newwin->windowArea.height_px);

    return newwin;
}

void kcompose_flush(void)
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_GRAPHICS_READY);
    KGraphicsArea* backbuffer = (KGraphicsArea*)&g_kstate.gx_backfb;
    kgraphics_rect (backbuffer, 0, 0, backbuffer->width_px, backbuffer->height_px,
                    DESKTOP_BG_COLOR);

    ListNode* node;
    list_for_each (&windowsListHead, node)
    {
        Window* win = LIST_ITEM (node, Window, windowListNode);
        UINT fby    = win->position.screen_y;
        UINT fbx    = win->position.screen_x;
        kgraphics_blit (backbuffer, fbx, fby, &win->windowArea);
    }
}
