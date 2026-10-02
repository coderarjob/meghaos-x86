#include <syscall.h>
#include <types.h>
#include <cm.h>
#include <graphics.h>
#include <debug.h>

#define SINGLE_THREADED

void thread0 (void);
void thread1 (void);
void thread2 (void);
#ifdef SINGLE_THREADED
void singlethread (void);
#endif

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

static OSIF_WindowFrameBufferInfo fbi0;
static OSIF_WindowFrameBufferInfo fbi1;
static OSIF_WindowFrameBufferInfo fbi2;

void proc_main (void)
{
    cm_window_flush_graphics();
#ifdef SINGLE_THREADED
    cm_thread_create (&singlethread, false);
#else
    cm_thread_create (&thread0, false);
    cm_thread_create (&thread1, false);
    cm_thread_create (&thread2, false);
#endif

    cm_process_register_event_handler (OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ, repaint_on_yield);

    fbi0 = createWindow ("gui0 - Window 1");
    fbi1 = createWindow ("gui0 - Window 2");
    fbi2 = createWindow ("gui0 - Window 3");

    while (1) {
        cm_process_handle_events();
    }
}

#ifdef SINGLE_THREADED
void singlethread (void)
{
    while (1) {
        thread0();
        thread1();
        thread2();
    }
}
#endif

void thread0 (void)
{

    static int value = 1;
    static int incby = 1;
#ifndef SINGLE_THREADED
    while (1)
#endif
    {
        CM_DBG_INFO("thread0 - Started");
        triangle_sum (&value, 1, 10, &incby);

        for (unsigned int x = 0; x < fbi0.width_px; x++) {
            for (unsigned int y = 0; y < fbi0.height_px; y++) {
                UINT color = ((x) * (UINT)value & 255) << 0;
                graphics_putpixel (&fbi0, x, y, color);
            }
        }
        cm_delay (10);
    }
}

void thread1 (void)
{
    static int value = 1;
    static int incby = 1;
#ifndef SINGLE_THREADED
    while (1)
#endif
    {
        triangle_sum (&value, 10, 20, &incby);

        for (unsigned int x = 0; x < fbi1.width_px; x++) {
            for (unsigned int y = 0; y < fbi1.height_px; y++) {
                UINT color = ((y) * (UINT)value & 255) << 16;
                graphics_putpixel (&fbi1, x, y, color);
            }
        }
        cm_delay (10);
    }
}

void thread2 (void)
{
    static int value = 1;
    static int incby = 1;
#ifndef SINGLE_THREADED
    while (1)
#endif
    {
        triangle_sum (&value, 1, 10, &incby);

        for (unsigned int x = 0; x < fbi2.width_px; x++) {
            for (unsigned int y = 0; y < fbi2.height_px; y++) {
                UINT color = ((x + y) * (UINT)value & 255) << 8;
                graphics_putpixel (&fbi2, x, y, color);
            }
        }
        cm_delay (10);
    }
}
