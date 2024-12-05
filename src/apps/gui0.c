#include <syscall.h>
#include <types.h>
#include <cm.h>
#include <graphics.h>
#include <debug.h>

void thread0();
void thread1();
void thread2();

static OSIF_WindowFrameBufferInfo createWindow (const char* const title)
{
    Handle h = cm_window_create (title);
    if (h == INVALID_HANDLE) {
        CM_DBG_ERROR ("Window creation failed");
        HALT();
    }

    OSIF_WindowFrameBufferInfo fbi;
    if (!cm_window_getFB (h, &fbi)) {
        CM_DBG_ERROR ("Window creation failed");
        HALT();
    }

    return fbi;
}

static void triangle_sum (int* now, int start, int end, int* incby)
{
    int l_now = *now;

    l_now += *incby;

    if (l_now >= end) {
        l_now = end;
    } else if (l_now <= start) {
        l_now = start;
    }

    if (l_now == end || l_now == start) {
        *incby *= -1;
    }

    *now = l_now;
}

static void repaint_on_yield (OSIF_ProcessEvent const* const e)
{
    (void)e;
    cm_window_flush_graphics();
}

void proc_main()
{
    cm_window_flush_graphics();
    cm_thread_create (&thread0, false);
    cm_thread_create (&thread1, false);
    cm_thread_create (&thread2, false);

    cm_process_register_event_handler (OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ, repaint_on_yield);

    while (1) {
        cm_process_handle_events();
    }
}

void thread0()
{
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 1");
    int value                      = 1;
    int incby                      = 1;
    while (1) {
        triangle_sum (&value, 1, 10, &incby);

        for (unsigned int x = 0; x < fbi.width_px; x++) {
            for (unsigned int y = 0; y < fbi.height_px; y++) {
                UINT color = ((x) * (UINT)value & 255) << 0;
                graphics_putpixel (&fbi, x, y, color);
            }
        }
        cm_delay (50);
    }
}

void thread1()
{
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 2");
    int value                      = 10;
    int incby                      = 1;
    while (1) {
        triangle_sum (&value, 10, 20, &incby);

        for (unsigned int x = 0; x < fbi.width_px; x++) {
            for (unsigned int y = 0; y < fbi.height_px; y++) {
                UINT color = ((y) * (UINT)value & 255) << 16;
                graphics_putpixel (&fbi, x, y, color);
            }
        }
        cm_delay (50);
    }
}

void thread2()
{
    OSIF_WindowFrameBufferInfo fbi = createWindow ("gui0 - Window 3");
    int value                      = 1;
    int incby                      = 1;
    while (1) {
        triangle_sum (&value, 1, 10, &incby);

        for (unsigned int x = 0; x < fbi.width_px; x++) {
            for (unsigned int y = 0; y < fbi.height_px; y++) {
                UINT color = ((x + y) * (UINT)value & 255) << 8;
                graphics_putpixel (&fbi, x, y, color);
            }
        }
        cm_delay (80);
    }
}
