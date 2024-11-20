#include <syscall.h>
#include <types.h>
#include <app.h>
#include <graphics.h>

void thread0();
void thread1();
void thread2();

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
    os_thread_create (&thread1, false);
    os_thread_create (&thread2, false);
    os_thread_create (&thread0, false);

    while (1) {
        if (os_process_is_yield_requested()) {
            os_yield();
        }
    }
}

void thread0()
{
    UINT color                     = 0x5F9FFF;
    INT x                          = 0;
    INT width                      = 20;
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 3");
    graphics_rect (&fbi, 0, 0, fbi.width_px, fbi.height_px, 0x0);
    while (1) {
        if ((UINT)(x + width) >= fbi.width_px) {
            x = 0;
            graphics_rect (&fbi, 0, 0, fbi.width_px, fbi.height_px, 0x0);
        }
        graphics_rect (&fbi, (UINT)x, 0, (UINT)width, fbi.height_px, color);
        x += width + 3;

        os_window_flush_graphics();
        delay (500);
    }
}

void thread1()
{
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 1");
    int modby                      = 1;
    int incby                      = 1;
    while (1) {
        if (modby == 10) {
            incby = -1;
        } else if (modby == 1) {
            incby = 1;
        }
        modby += incby;

        for (unsigned int x = 0; x < fbi.width_px; x++) {
            for (unsigned int y = 0; y < fbi.height_px; y++) {
                UINT color = (20 * (UINT)modby & 255) << 16;
                graphics_putpixel (&fbi, x, y, color);
            }
        }
        os_window_flush_graphics();
        delay (100);
    }
}

void thread2()
{
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 2");
    int modby                      = 1;
    int incby                      = 1;
    while (1) {
        if (modby == 10) {
            incby = -1;
        } else if (modby == 1) {
            incby = 1;
        }
        modby += incby;

        for (unsigned int x = 0; x < fbi.width_px; x++) {
            for (unsigned int y = 0; y < fbi.height_px; y++) {
                UINT color = ((x + y) * (UINT)modby & 255) << 8;
                graphics_putpixel (&fbi, x, y, color);
            }
        }
        os_window_flush_graphics();
        delay (80);
    }
}
