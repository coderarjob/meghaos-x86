/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - OS Syscall calls
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#if defined(LIBCM) || defined (UNITTEST)
    #include <cm/osif.h>
#else
    #include <osif.h>
#endif

S32 syscall (OSIF_SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
UINT cm_get_tick_period_us(void);

static inline UINT cm_tickcount_to_microsec(UINT tick)
{
    return ((tick)*cm_get_tick_period_us());
}

static inline void cm_putstr (char* text)
{
    syscall (OSIF_SYSCALL_CONSOLE_WRITELN, (PTR)text, 0, 0, 0, 0);
}

static inline U32 cm_get_tickcount(void)
{
    return (U32)syscall (OSIF_SYSCALL_TIMER_GET_TICKCOUNT, 0, 0, 0, 0, 0);
}

static inline Handle cm_window_create (const char* title)
{
    return (Handle)syscall (OSIF_SYSCALL_WINDOW_CREATE, (PTR)title, 0, 0, 0, 0);
}

static inline U32 cm_window_destory (Handle h)
{
    return (U32)syscall (OSIF_SYSCALL_WINDOW_DESTORY, (U32)h, 0, 0, 0, 0);
}

static inline void* cm_window_flush_graphics(void)
{
    return (void*)syscall (OSIF_SYSCALL_WINDOW_FLUSH_GRAPHICS, 0, 0, 0, 0, 0);
}

static inline bool cm_window_getFB (Handle h, OSIF_WindowFrameBufferInfo* wfb)
{
    return syscall (OSIF_SYSCALL_WINDOW_GET_WINDOW_FB, (U32)h, (PTR)wfb, 0, 0, 0);
}
