#include <syscall.h>
#include <types.h>
#define INVALID_HANDLE -1

#ifndef GRAPHICS_MODE_ENABLED
    #error "Gui demo only works in graphics mode"
#endif

void proc_main()
{
    Handle h = os_window_create ("gui0 - 1");
    if (h == INVALID_HANDLE) {
        os_putstr ("Window creation failed");
        for (;;)
            ;
    }
    OSIF_WindowFrameBufferInfo fb;
    if (!os_window_getFB (h, &fb)) {
        os_putstr ("Window creation failed");
        for (;;)
            ;
    }
    U8* buffer = fb.buffer;
    __builtin_memset (buffer, 0x4, 400 * 30);
    os_window_flush_graphics();
    for (;;)
        ;
}
