/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - VGA Routines etc.
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/
#ifndef __VGA_COLORS_H__
#define __VGA_COLORS_H__

enum vga_colors {BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY, 
                  DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED, 
                  PINK, YELLOW, WHITE};

#define disp_attr(bg,fg,b) ((b)<<7|(bg)<<4|(fg))

#endif // __VGA_COLORS_H__
