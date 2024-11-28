/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - OS Syscall calls
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
// App library is built with the with Kernel flags and kernel include directories. Whereas when
// applications will include this header (either directly or indirectly) the include path will
// differ, thus this change in the way applib headers are included here.
#ifdef KERNEL
    #include <cm/osif.h>
#else
    #include <osif.h>
#endif

S32 syscall (OSIF_SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
INT cm_thread_create (void (*startLocation)(), bool isKernelMode);
INT cm_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode);
UINT cm_get_tick_period_us();
bool cm_process_is_yield_requested();
bool cm_process_is_child_exited (UINT* exitCode);

static inline UINT cm_tickcount_to_microsec(UINT tick)
{
    return ((tick)*cm_get_tick_period_us());
}

static inline void cm_process_yield()
{
    syscall (OSIF_SYSCALL_YIELD_PROCESS, 0, 0, 0, 0, 0);
}

static inline void cm_process_kill(UINT code)
{
    syscall (OSIF_SYSCALL_KILL_PROCESS, code, 0, 0, 0, 0);
}

static inline void cm_putstr (char* text)
{
    syscall (OSIF_SYSCALL_CONSOLE_WRITELN, (PTR)text, 0, 0, 0, 0);
}

static inline U32 cm_process_get_pid()
{
    return (U32)syscall (OSIF_SYSCALL_PROCESS_GETPID, 0, 0, 0, 0, 0);
}

static inline U32 cm_get_tickcount()
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

static inline void* cm_process_get_datamem_start()
{
    return (void*)syscall (OSIF_SYSCALL_PROCESS_GET_DATAMEM_START, 0, 0, 0, 0, 0);
}

static inline void* cm_window_flush_graphics()
{
    return (void*)syscall (OSIF_SYSCALL_WINDOW_FLUSH_GRAPHICS, 0, 0, 0, 0, 0);
}

static inline bool cm_window_getFB (Handle h, OSIF_WindowFrameBufferInfo* wfb)
{
    return syscall (OSIF_SYSCALL_WINDOW_GET_WINDOW_FB, (U32)h, (PTR)wfb, 0, 0, 0);
}

static inline bool cm_process_pop_event (OSIF_ProcessEvent* e)
{
    return syscall (OSIF_SYSCALL_POP_PROCESS_EVENT, (PTR)e, 0, 0, 0, 0);
}
