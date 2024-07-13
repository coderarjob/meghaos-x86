/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - errno and error reporting
* routines.
* ---------------------------------------------------------------------------
*
* Dated: 9th November 2020
*/

#ifndef ERRORNO_H
#define ERRORNO_H

#include <types.h>
#include <kassert.h>
#include <kdebug.h>
#include <panic.h>
#include <x86/kernel.h>

/* Error codes that can be set inside the kernel.*/
typedef enum KernelErrorCodes {
    ERR_NONE                      = 0,  // No error
    ERR_UNKNOWN                   = 1,  // Unknown error
    ERR_INVALID_RANGE             = 2,  // Outside of valid range
    ERR_OVERFLOW                  = 3,  // Overflow
    ERR_OUT_OF_MEM                = 4,  // Out of physical pages
    ERR_DOUBLE_FREE               = 5,  // Double free
    ERR_DOUBLE_ALLOC              = 6,  // Double allocation
    ERR_WRONG_ALIGNMENT           = 7,  // Byte alignment is wrong
    ERR_OUTSIDE_ADDRESSABLE_RANGE = 8,  // Byte is outside addressable range.
    ERR_INVALID_ARGUMENT          = 9,  // Invalid argument.",
    ERR_PAGE_WRONG_STATE          = 10, // Page table/directory is in wrong state
    ERR_SCHEDULER_QUEUE_FULL      = 11, // No space for new process in process table
    ERR_SCHEDULER_QUEUE_EMPTY     = 12, // No process in the process table to schedule next
    ERR_VMM_NOT_ALLOCATED         = 13, // Virutal address space is not allocated
    ERR_VMM_NOT_RESERVED          = 14, // Virtual address space is not reserved
} KernelErrorCodes;

// Use this with RETURN_ERROR when you do not want to set a new error number but pass through what
// is already set.
#define ERROR_PASSTHROUGH g_kstate.errorNumber

#define KERNEL_EXIT_SUCCESS  0
#define KERNEL_EXIT_FAILURE -1

/* Can be used to store an error code and return from a function */
#define RETURN_ERROR(errno, rval)     \
    do {                              \
        ERROR ("Error %x.", errno);   \
        g_kstate.errorNumber = errno; \
        return rval;                  \
    } while (0)

/* Displays error description if error code != 0 */
#define k_assertOnError() k_assert (g_kstate.errorNumber == ERR_NONE, "Assert on error")
#define k_panicOnError()                                                 \
    do {                                                                 \
        if (g_kstate.errorNumber != ERR_NONE) {                          \
            k_panic ("Panic on error.Error %x\n", g_kstate.errorNumber); \
        }                                                                \
    } while (0)

#endif // ERRORNO_H
