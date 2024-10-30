/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - OS, application library interfaces
 * -------------------------------------------------------------------------------------------------
 */

#pragma once
#include <types.h>

typedef struct OSIF_WindowFrameBufferInfo {
    UINT width_px;
    UINT height_px;
    UINT bytesPerPixel;
    UINT bytesPerRow;
    U8* buffer;
    SIZE bufferSizeBytes;
} OSIF_WindowFrameBufferInfo;

typedef enum OSIF_SYSCALLS {
    SYSCALL_CONSOLE_WRITELN           = 0,
    SYSCALL_CREATE_PROCESS            = 1,
    SYSCALL_YIELD_PROCESS             = 2,
    SYSCALL_KILL_PROCESS              = 3,
    SYSCALL_CONSOLE_SETCOLOR          = 4,
    SYSCALL_CONSOLE_SETCURSORPOS      = 5,
    SYSCALL_POP_PROCESS_EVENT         = 6,
    SYSCALL_PROCESS_GETPID            = 7,
    SYSCALL_TIMER_GET_TICKCOUNT       = 8,
    SYSCALL_PROCESS_GET_DATAMEM_START = 9,
    SYSCALL_WINDOW_CREATE             = 10,
    SYSCALL_WINDOW_DESTORY            = 11,
    SYSCALL_WINDOW_GET_WINDOW_FB      = 12,
    SYSCALL_WINDOW_FLUSH_GRAPHICS     = 13,
} OSIF_SYSCALLS;
