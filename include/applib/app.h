/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

typedef struct ProcessEvent {
    UINT event;
    UINT data;
} ProcessEvent;

typedef enum AppEvents {
    APP_EVENT_NONE               = 0,
    APP_EVENT_PROCCESS_YIELD_REQ = 2
} AppEvents;
// =============================================================================

typedef enum SYSCALLS {
    SYSCALL_CONSOLE_WRITELN      = 0,
    SYSCALL_CREATE_PROCESS       = 1,
    SYSCALL_YIELD_PROCESS        = 2,
    SYSCALL_KILL_PROCESS         = 3,
    SYSCALL_CONSOLE_SETCOLOR     = 4,
    SYSCALL_CONSOLE_SETCURSORPOS = 5,
    SYSCALL_POP_PROCESS_EVENT    = 6,
    SYSCALL_PROCESS_GETPID       = 7,
    SYSCALL_TIMER_GET_TICKCOUNT  = 8,
} SYSCALLS;

S32 syscall (SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
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
