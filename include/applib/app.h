/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <applib/osif.h>

typedef struct ProcessEvent {
    UINT event;
    UINT data;
} ProcessEvent;

typedef enum AppEvents {
    APP_EVENT_NONE               = 0,
    APP_EVENT_PROCCESS_YIELD_REQ = 2
} AppEvents;
// =============================================================================

S32 syscall (OSIF_SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
INT sys_thread_create (void (*startLocation)(), bool isKernelMode);
INT sys_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode);
UINT os_tick_microseconds();
bool sys_process_pop_event (U32 pid, ProcessEvent* e);

static inline void sys_yield()
{
    syscall (SYSCALL_YIELD_PROCESS, 0, 0, 0, 0, 0);
}

static inline void sys_process_kill()
{
    syscall (SYSCALL_KILL_PROCESS, 0, 0, 0, 0, 0);
}

static inline void sys_putstr (char* text)
{
    syscall (SYSCALL_CONSOLE_WRITELN, (PTR)text, 0, 0, 0, 0);
}

static inline U32 sys_process_get_pid()
{
    return (U32)syscall (SYSCALL_PROCESS_GETPID, 0, 0, 0, 0, 0);
}

static inline U32 sys_get_tickcount()
{
    return (U32)syscall (SYSCALL_TIMER_GET_TICKCOUNT, 0, 0, 0, 0, 0);
}

static inline Handle sys_window_create (const char* title)
{
    return (Handle)syscall (SYSCALL_WINDOW_CREATE, (PTR)title, 0, 0, 0, 0);
}

static inline U32 sys_window_destory (Handle h)
{
    return (U32)syscall (SYSCALL_WINDOW_DESTORY, (U32)h, 0, 0, 0, 0);
}

static inline void* sys_process_get_datamem_start()
{
    return (void*)syscall (SYSCALL_PROCESS_GET_DATAMEM_START, 0, 0, 0, 0, 0);
}

static inline void* sys_window_flush_graphics()
{
    return (void*)syscall (SYSCALL_WINDOW_FLUSH_GRAPHICS, 0, 0, 0, 0, 0);
}

static inline bool sys_window_getFB (Handle h, OSIF_WindowFrameBufferInfo* wfb)
{
    return syscall (SYSCALL_WINDOW_GET_WINDOW_FB, (U32)h, (PTR)wfb, 0, 0, 0);
}
