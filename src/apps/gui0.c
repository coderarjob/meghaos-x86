#include <syscall.h>
#include <types.h>
#include <app.h>
#include <graphics.h>

#ifndef GRAPHICS_MODE_ENABLED
    #error "Gui demo only works in graphics mode"
#endif

void thread0();
void thread1();

bool os_process_is_yield_requested()
{
    UINT pid            = os_process_get_pid();
    OSIF_ProcessEvent e = { 0 };
    os_process_pop_event (pid, &e);
    return (e.event == OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ);
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
    Handle h = os_window_create ("window - 0");
    if (h == INVALID_HANDLE) {
        ERROR ("Window creation failed");
        HALT();
    }

    OSIF_WindowFrameBufferInfo fbi;
    if (!os_window_getFB (h, &fbi)) {
        ERROR ("Window creation failed");
        HALT();
    }

    UINT x     = 10;
    UINT color = 0x20;
    while (1) {
        graphics_rect (&fbi, x, x, 100, 200, color);
        graphics_rect (&fbi, x + 100 + 10, x, 100, 200, color);
        // delay(50);
        // x+=5;
        if (color++ > 0x30) {
            color = 0x20;
        }

        if (os_process_is_yield_requested()) {
            os_window_flush_graphics();
            os_yield();
        }
    }
}

void thread1()
{
    Handle h = os_window_create ("window - 1");
    if (h == INVALID_HANDLE) {
        ERROR ("Window creation failed");
        HALT();
    }

    OSIF_WindowFrameBufferInfo fbi;
    if (!os_window_getFB (h, &fbi)) {
        ERROR ("Window creation failed");
        HALT();
    }

    UINT x     = 10;
    UINT color = 0x70;
    while (1) {
        graphics_rect (&fbi, x, x, 200, 100, color);
        graphics_rect (&fbi, x, x + 100 + 10, 200, 100, color);
        if (color++ > 0x80) {
            color = 0x70;
        }

        if (os_process_is_yield_requested()) {
            os_window_flush_graphics();
            os_yield();
        }
    }
}
