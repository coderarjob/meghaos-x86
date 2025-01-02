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
#if defined(LIBCM) || defined(UNITTEST)
    #include <cm/osif.h>
    #include <cm/syscall.h>
#else
    #include <osif.h>
    #include <syscall.h>
#endif

#define INVALID_HANDLE     (-1)
#define CM_ABORT_EXIT_CODE (127U)

/***************************************************************************************************
 * Misc functions
 ***************************************************************************************************/
#define HALT()             for (;;)

#ifndef UNITTEST
    #define cm_panic()                                            \
        do {                                                      \
            CM_DBG_ERROR ("Panic at %s: %u", __FILE__, __LINE__); \
            cm_process_abort (CM_ABORT_EXIT_CODE);                \
        } while (0)
#else // UNITTEST
    void cm_unittest_panic_handler();
    extern bool cm_panic_invoked;

    /* Returns from the 'function under testing', when an assert/panic is hit.
     *
     * There is x86 assembly hard coded in an arch independent header, however this corresponds to
     * the host (not the target) arch. Which implies that unittests can only be built & run on an
     * x86 machine.
     *
     * TODO: Find some way to make this host independent.
     * NOTE: EAX is not preserved by GCC. So there is not point adding it to the clobber list.
     */
    #define cm_panic()                                                      \
        do {                                                                \
            cm_unittest_panic_handler();                                    \
            __asm__ volatile("mov esp, ebp; pop ebp; mov eax, 0; ret;" ::); \
        } while (0)
#endif // UNITTEST

#if defined(DEBUG)
    #define cm_assert(t)    \
        do {                \
            if (!(t)) {     \
                cm_panic(); \
            }               \
        } while (0)
#else
    #define cm_assert(...) (void)0
#endif // DEBUG

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
INT cm_process_create (const char* const filename, bool isKernelMode);

static inline bool cm_process_pop_event (OSIF_ProcessEvent* e)
{
    return syscall (OSIF_SYSCALL_POP_PROCESS_EVENT, (PTR)e, 0, 0, 0, 0);
}

static inline void cm_process_yield()
{
    syscall (OSIF_SYSCALL_YIELD_PROCESS, 0, 0, 0, 0, 0);
}

static inline void cm_process_kill (UINT code)
{
    syscall (OSIF_SYSCALL_KILL_PROCESS, code, 0, 0, 0, 0);
}

static inline void cm_process_abort (UINT code)
{
    syscall (OSIF_SYSCALL_ABORT_PROCESS, code, 0, 0, 0, 0);
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
INT cm_snprintf (CHAR* dest, size_t size, const CHAR* fmt, ...);
INT cm_vsnprintf (CHAR* dest, size_t size, const CHAR* fmt, va_list l);

/***************************************************************************************************
 * Heap managemnt
 ***************************************************************************************************/
void cm_malloc_init();
void* cm_malloc (size_t bytes);
void* cm_calloc (size_t bytes);
bool cm_free (void* addr);
