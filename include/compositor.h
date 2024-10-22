/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Window compositor header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

#define WINDOW_MANAGER_GRID_ROWS_MAX    (2U)
#define WINDOW_MANAGER_GRID_COLS_MAX    (2U)
#define WINDOW_MANAGER_GRID_CELL_COUNT  (WINDOW_MANAGER_GRID_ROWS_MAX * WINDOW_MANAGER_GRID_COLS_MAX)

#define WINDOW_MANAGER_GRID_WIDTH(gx)   (gx.xResolution / WINDOW_MANAGER_GRID_COLS_MAX)
#define WINDOW_MANAGER_GRID_HEIGHT(gx)  (gx.yResolution / WINDOW_MANAGER_GRID_ROWS_MAX)
#define WINDOW_MANAGER_GRID_TOP(gx, r)  (WINDOW_MANAGER_GRID_HEIGHT (gx) * r)
#define WINDOW_MANAGER_GRID_LEFT(gx, c) (WINDOW_MANAGER_GRID_WIDTH (gx) * c)

void kcompose_init();
void kcompose_flush();
