/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Window compositor header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#ifndef GRAPHICS_MODE_ENABLED
    #error "Compositor only works in graphics mode"
#endif

#include <kernel.h>
#include <types.h>
#include <kgraphics.h>

typedef struct Window {
    struct {
        UINT screen_x;
        UINT screen_y;
    } position;
    KGraphicsArea windowArea;
    KGraphicsArea workingArea;
    ListNode windowListNode;
    UINT processID;
} Window;

#define WINMAN_GRID_ROWS_MAX   (2U)
#define WINMAN_GRID_COLS_MAX   (2U)
#define WINMAN_GRID_CELL_COUNT (WINMAN_GRID_ROWS_MAX * WINMAN_GRID_COLS_MAX)

#define WINMAN_GRID_CELL_WIDTH_PX()         (g_kstate.gx_backfb.width_px / WINMAN_GRID_COLS_MAX)
#define WINMAN_GRID_CELL_HEIGHT_PX()        (g_kstate.gx_backfb.height_px / WINMAN_GRID_ROWS_MAX)

// Window grid coordiate of rows and columns to screen coordinates
#define WINMAN_GRID_CELL_Y(r)               (WINMAN_GRID_CELL_HEIGHT_PX() * r)
#define WINMAN_GRID_CELL_X(c)               (WINMAN_GRID_CELL_WIDTH_PX() * c)

// Window grid index to window grid coordiate of rows and columns
#define WINMAN_GRID_CELL_ROW(wini)          (wini / WINMAN_GRID_COLS_MAX)
#define WINMAN_GRID_CELL_COL(wini)          (wini % WINMAN_GRID_COLS_MAX)

// Window grid coordiate of rows and columns to Window grid index
#define WINMAN_WININDEX_FROM_CELL_POS(r, c) (r * WINMAN_GRID_COLS_MAX + c)

void kcompose_init(void);
void kcompose_flush(void);
Window* kcompose_createWindow (const char* const title);
bool kcompose_destroyWindow (Window* win);
