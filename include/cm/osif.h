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
    OSIF_SYSCALL_CONSOLE_WRITELN           = 0,
    OSIF_SYSCALL_CREATE_PROCESS            = 1,
    OSIF_SYSCALL_YIELD_PROCESS             = 2,
    OSIF_SYSCALL_KILL_PROCESS              = 3,
    OSIF_SYSCALL_CONSOLE_SETCOLOR          = 4,
    OSIF_SYSCALL_CONSOLE_SETCURSORPOS      = 5,
    OSIF_SYSCALL_POP_PROCESS_EVENT         = 6,
    OSIF_SYSCALL_PROCESS_GETPID            = 7,
    OSIF_SYSCALL_TIMER_GET_TICKCOUNT       = 8,
    OSIF_SYSCALL_PROCESS_GET_DATAMEM_START = 9,
    OSIF_SYSCALL_WINDOW_CREATE             = 10,
    OSIF_SYSCALL_WINDOW_DESTORY            = 11,
    OSIF_SYSCALL_WINDOW_GET_WINDOW_FB      = 12,
    OSIF_SYSCALL_WINDOW_FLUSH_GRAPHICS     = 13,
    OSIF_SYSCALL_GET_OS_ERROR              = 14,
    OSIF_SYSCALL_GET_BOOTLOADED_FILE       = 15,
    OSIF_SYSCALL_ABORT_PROCESS             = 16,
    OSIF_SYSCALL_TEST                      = 17,
} OSIF_SYSCALLS;

typedef enum OSIF_ProcessEvents {
    OSIF_PROCESS_EVENT_NONE                  = 0,
    OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ    = 1,
    OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED = 2,
    OSIF_PROCESS_EVENTS_COUNT
} OSIF_ProcessEvents;

typedef struct OSIF_ProcessEvent {
    OSIF_ProcessEvents event;
    U64 data;
} OSIF_ProcessEvent;

typedef struct OSIF_BootLoadedFiles {
    void* startLocation;
    U16 length;
} OSIF_BootLoadedFiles;
