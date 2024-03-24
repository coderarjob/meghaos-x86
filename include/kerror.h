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

/* Error codes that can be set inside the kernel.*/
typedef enum KernelErrorCodes {
    ERR_NONE                      = 0, // No error
    ERR_UNKNOWN                   = 1, // Unknown error
    ERR_INVALID_RANGE             = 2, // Outside of valid range
    ERR_OVERFLOW                  = 3, // Overflow
    ERR_OUT_OF_MEM                = 4, // Out of physical pages
    ERR_DOUBLE_FREE               = 5, // Double free
    ERR_DOUBLE_ALLOC              = 6, // Double allocation
    ERR_WRONG_ALIGNMENT           = 7, // Byte alignment is wrong
    ERR_OUTSIDE_ADDRESSABLE_RANGE = 8, // Byte is outside addressable range.
    ERR_INVALID_ARGUMENT          = 9, // Invalid argument.",
    ERR_PAGE_WRONG_STATE          = 10 // Page table/directory is in wrong state
} KernelErrorCodes;

/* This variable is globally used to set error codes*/
extern KernelErrorCodes k_errorNumber;

/* Can be used to set the k_errorNumber global and return from a function */
#define RETURN_ERROR(errno, rval)                                                            \
            do {                                                                             \
                   ERROR ("Error 0x%x.", errno);                                             \
                   k_errorNumber = errno;                                                    \
                   return rval;                                                              \
                }while(0)

#define KERNEL_EXIT_SUCCESS  0
#define KERNEL_EXIT_FAILURE -1

/* Displays error description if k_errorNumber != 0 */
#define k_assertOnError() k_assert (k_errorNumber == ERR_NONE, "Assert on error")
#define k_panicOnError()                                            \
    do {                                                            \
        if (k_errorNumber != ERR_NONE) {                            \
            k_panic ("Panic on error.Error 0x%x\n", k_errorNumber); \
        }                                                           \
    } while (0)

#endif // ERRORNO_H
