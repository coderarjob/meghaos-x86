/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - C MOS App library - Library errors
 * ---------------------------------------------------------------------------
 */

#include <stdint.h>
#if defined(KERNEL) || defined (UNITTEST)
    #include <cm/debug.h>
    #include <cm/syscall.h>
    #include <cm/osif.h>
#else
    #include <debug.h>
    #include <syscall.h>
    #include <osif.h>
#endif

#define CM_FAILURE (-1)

typedef enum CMErrors {
    // OS Errors start from 0 and must end before the start of library errors
    LIBRARY_ERRORS_START                    = 100,
    CM_ERR_INVALID_INPUT                    = 100,
    CM_ERR_EVENT_HANDLER_ALREADY_REGISTERED = 101,
} CMErrors;

extern uint32_t cm_error_num__;

static inline uint32_t cm_get_lib_error()
{
    return cm_error_num__;
}

static inline uint32_t cm_get_os_error()
{
    return (uint32_t)syscall (OSIF_SYSCALL_GET_OS_ERROR, 0, 0, 0, 0, 0);
}

/* Can be used to store an error code and return from a function */
#define CM_RETURN_ERROR__(errno, rval)     \
    do {                                   \
        CM_DBG_ERROR ("Error %x.", errno); \
        cm_error_num__ = errno;            \
        return rval;                       \
    } while (0)
