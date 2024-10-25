/*
 * ---------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - GUI colors
 * ---------------------------------------------------------------------------------------
 */

#pragma once

#if CONFIG_GXMODE_BITSPERPIXEL == 8
    #define COLOR_WHITE               (15)
    #define WINDOW_TITLE_BAR_BG_COLOR (126)
    #define WINDOW_TITLE_BAR_FG_COLOR (15)
    #define WINDOW_BG_COLOR           (29)
    #define WINDOW_BORDER_COLOR1      (26)
    #define WINDOW_BORDER_COLOR2      (30)
    #define DESKTOP_BG_COLOR          (123)
    #define MOUSE_BG_COLOR            (18)
    #define MOUSE_FG_COLOR            (15)
#elif CONFIG_GXMODE_BITSPERPIXEL == 32 || CONFIG_GXMODE_BITSPERPIXEL == 24
    #define COLOR_WHITE               (0xFFFFFF)
    #define WINDOW_TITLE_BAR_BG_COLOR (0x003971)
    #define WINDOW_TITLE_BAR_FG_COLOR (0xFFFFFF)
    #define WINDOW_BG_COLOR           (0xDFDFDF)
    #define WINDOW_BORDER_COLOR1      (0xAAAAAA)
    #define WINDOW_BORDER_COLOR2      (0xF2F2F2)
    #define DESKTOP_BG_COLOR          (0x007155)
    #define MOUSE_BG_COLOR            (0x202020)
    #define MOUSE_FG_COLOR            (0xFFFFFF)
#endif
