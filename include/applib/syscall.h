/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <applib/osif.h>

S32 syscall (OSIF_SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
INT os_thread_create (void (*startLocation)(), bool isKernelMode);
INT os_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode);
UINT os_tick_microseconds();

static inline void os_yield()
{
    syscall (OSIF_SYSCALL_YIELD_PROCESS, 0, 0, 0, 0, 0);
}

static inline void os_process_kill()
{
    syscall (OSIF_SYSCALL_KILL_PROCESS, 0, 0, 0, 0, 0);
}

static inline void os_putstr (char* text)
{
    syscall (OSIF_SYSCALL_CONSOLE_WRITELN, (PTR)text, 0, 0, 0, 0);
}

static inline U32 os_process_get_pid()
{
    return (U32)syscall (OSIF_SYSCALL_PROCESS_GETPID, 0, 0, 0, 0, 0);
}

static inline U32 os_get_tickcount()
{
    return (U32)syscall (OSIF_SYSCALL_TIMER_GET_TICKCOUNT, 0, 0, 0, 0, 0);
}

static inline Handle os_window_create (const char* title)
{
    return (Handle)syscall (OSIF_SYSCALL_WINDOW_CREATE, (PTR)title, 0, 0, 0, 0);
}

static inline U32 os_window_destory (Handle h)
{
    return (U32)syscall (OSIF_SYSCALL_WINDOW_DESTORY, (U32)h, 0, 0, 0, 0);
}

static inline void* os_process_get_datamem_start()
{
    return (void*)syscall (OSIF_SYSCALL_PROCESS_GET_DATAMEM_START, 0, 0, 0, 0, 0);
}

static inline void* os_window_flush_graphics()
{
    return (void*)syscall (OSIF_SYSCALL_WINDOW_FLUSH_GRAPHICS, 0, 0, 0, 0, 0);
}

static inline bool os_window_getFB (Handle h, OSIF_WindowFrameBufferInfo* wfb)
{
    return syscall (OSIF_SYSCALL_WINDOW_GET_WINDOW_FB, (U32)h, (PTR)wfb, 0, 0, 0);
}

static inline bool os_process_pop_event (U32 pid, OSIF_ProcessEvent* e)
{
    return syscall (OSIF_SYSCALL_POP_PROCESS_EVENT, pid, (PTR)e, 0, 0, 0);
}
