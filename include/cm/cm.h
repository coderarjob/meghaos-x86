/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - App library - General macros & function declarations
 * ---------------------------------------------------------------------------
 *
 *  This file contains Mos-libc macros & definitions for application to use.
 */

#pragma once

#include <types.h>
#include <stdarg.h>
#if defined(KERNEL) || defined(UNITTEST)
    #include <cm/osif.h>
    #include <cm/syscall.h>
#else
    #include <osif.h>
    #include <syscall.h>
#endif

#define INVALID_HANDLE (-1)
#define HALT()         for (;;)

INT snprintf (CHAR* dest, size_t size, const CHAR* fmt, ...);
INT vsnprintf (CHAR* dest, size_t size, const CHAR* fmt, va_list l);

/***************************************************************************************************
 * Misc functions
 ***************************************************************************************************/
void cm_delay (UINT ms);

static inline void cm_get_bootloaded_file (const char* const filename,
                                           OSIF_BootLoadedFiles* const out)
{
    syscall (OSIF_SYSCALL_GET_BOOTLOADED_FILE, (U32)filename, (U32)out, 0, 0, 0);
}

/***************************************************************************************************
 * Process management
 ***************************************************************************************************/
INT cm_thread_create (void (*startLocation)(), bool isKernelMode);
INT cm_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode);

static inline void cm_process_yield()
{
    syscall (OSIF_SYSCALL_YIELD_PROCESS, 0, 0, 0, 0, 0);
}

static inline void cm_process_kill (UINT code)
{
    syscall (OSIF_SYSCALL_KILL_PROCESS, code, 0, 0, 0, 0);
}

static inline U32 cm_process_get_pid()
{
    return (U32)syscall (OSIF_SYSCALL_PROCESS_GETPID, 0, 0, 0, 0, 0);
}

static inline void* cm_process_get_datamem_start()
{
    return (void*)syscall (OSIF_SYSCALL_PROCESS_GET_DATAMEM_START, 0, 0, 0, 0, 0);
}

/***************************************************************************************************
 * Handling of process events
***************************************************************************************************/
typedef void (*cm_event_handler)(OSIF_ProcessEvent const * const);
bool cm_process_register_event_handler(OSIF_ProcessEvents event, cm_event_handler h);
bool cm_process_handle_events();

/***************************************************************************************************
 * String and memory functions
 ***************************************************************************************************/
void* cm_memcpy (void* dest, const void* src, size_t n);
UINT cm_strlen (const char* s);
char* cm_strncpy (char* d, const char* s, SIZE n);
void* cm_memset (void* const s, U8 c, size_t n);
