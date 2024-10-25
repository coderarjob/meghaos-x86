/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Window compositor header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <graphics.h>
#include <handle.h>

#define WINMAN_GRID_ROWS_MAX                (2U)
#define WINMAN_GRID_COLS_MAX                (2U)
#define WINMAN_GRID_CELL_COUNT              (WINMAN_GRID_ROWS_MAX * WINMAN_GRID_COLS_MAX)

#define WINMAN_GRID_CELL_WIDTH_PX(gx)       (gx.width_px / WINMAN_GRID_COLS_MAX)
#define WINMAN_GRID_CELL_HEIGHT_PX(gx)      (gx.height_px / WINMAN_GRID_ROWS_MAX)

// Window grid coordiate of rows and columns to screen coordinates
#define WINMAN_GRID_CELL_Y(gx, r)           (WINMAN_GRID_CELL_HEIGHT_PX (gx) * r)
#define WINMAN_GRID_CELL_X(gx, c)           (WINMAN_GRID_CELL_WIDTH_PX (gx) * c)

// Window grid index to window grid coordiate of rows and columns
#define WINMAN_GRID_CELL_ROW(wini)          (wini / WINMAN_GRID_COLS_MAX)
#define WINMAN_GRID_CELL_COL(wini)          (wini % WINMAN_GRID_COLS_MAX)

// Window grid coordiate of rows and columns to Window grid index
#define WINMAN_WININDEX_FROM_CELL_POS(r, c) (r * WINMAN_GRID_COLS_MAX + c)

void kcompose_init();
void kcompose_flush();
Handle kcompose_create_window (char* title);
KGraphicsArea* kcompose_get_graphics (Handle h);
