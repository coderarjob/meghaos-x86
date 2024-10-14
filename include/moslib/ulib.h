/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

// =============================================================================
// Kernel-User space interfaces (structures and emums)
//
// These will be same/similar to the Kernel types, defined in the Kernel headers, but can contain
// some changes to them suitable for user space.
// =============================================================================

// Represents the 'ProcessEvent' struct defined in 'process.h', but with some changes.
typedef struct IProcessEvent {
    UINT event;
    UINT data;
} IProcessEvent;

// Represents the 'ProcessFlags' struct defined in 'process.h', but with some changes.
typedef enum IProcessFlags {
    IPROCESS_FLAGS_NONE           = 0,
    IPROCESS_FLAGS_KERNEL_PROCESS = (1 << 0),
    IPROCESS_FLAGS_THREAD         = (1 << 1),
} IProcessFlags;
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
} SYSCALLS;

S32 syscall (SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
INT sys_thread_create (void (*startLocation)(), bool isKernelMode);
INT sys_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode);

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

static inline bool sys_process_pop_event (U32 pid, IProcessEvent* e)
{
    return syscall (SYSCALL_POP_PROCESS_EVENT, pid, (PTR)e, 0, 0, 0);
}
