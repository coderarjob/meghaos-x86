#include <syscall.h>
#include <types.h>
#include <app.h>
#include <graphics.h>

#ifndef GRAPHICS_MODE_ENABLED
    #error "Gui demo only works in graphics mode"
#endif

void thread0();
void thread1();

static OSIF_WindowFrameBufferInfo createWindow (const char* const title)
{
    Handle h = os_window_create (title);
    if (h == INVALID_HANDLE) {
        ERROR ("Window creation failed");
        HALT();
    }

    OSIF_WindowFrameBufferInfo fbi;
    if (!os_window_getFB (h, &fbi)) {
        ERROR ("Window creation failed");
        HALT();
    }

    return fbi;
}

void proc_main()
{
    os_window_flush_graphics();
    os_thread_create (&thread0, false);
    os_thread_create (&thread1, false);

    while (1) {
        if (os_process_is_yield_requested()) {
            os_yield();
        }
    }
}

void thread0()
{
#if GRAPHICS_BPP == 8
    UINT color       = 0x9;
    const UINT WHITE = 0xF;
#elif GRAPHICS_BPP == 32 || GRAPHICS_BPP == 24
    UINT color       = 0x0000FF;
    const UINT WHITE = 0xFFFFFFF;
#endif
    INT x                          = 0;
    INT width                      = 50;
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 1");
    while (1) {
        if ((UINT)(x + width) >= fbi.width_px) {
            x = 0;
            graphics_rect (&fbi, 0, 0, fbi.width_px, fbi.height_px, WHITE);
        }
        graphics_rect (&fbi, (UINT)x, 0, (UINT)width, fbi.height_px, color);
        x += width + 3;
        delay (10);
        os_window_flush_graphics();
    }
}

void thread1()
{
#if GRAPHICS_BPP == 8
    UINT color       = 0x4;
    const UINT WHITE = 0xF;
#elif GRAPHICS_BPP == 32 || GRAPHICS_BPP == 24
    UINT color       = 0xFF0000;
    const UINT WHITE = 0xFFFFFFF;
#endif
    INT y                          = 0;
    INT height                     = 50;
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 1");
    while (1) {
        if ((UINT)(y + height) >= fbi.height_px) {
            y = 0;
            graphics_rect (&fbi, 0, 0, fbi.width_px, fbi.height_px, WHITE);
        }
        graphics_rect (&fbi, 0, (UINT)y, fbi.width_px, (UINT)height, color);
        y += height + 3;
        delay (10);
        os_window_flush_graphics();
    }
}
