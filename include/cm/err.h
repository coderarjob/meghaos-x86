/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - C MOS App library - Library errors
 * ---------------------------------------------------------------------------
 */

#include <stdint.h>
#if defined(LIBCM) || defined (UNITTEST)
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
    CM_ERR_OUT_OF_HEAP_MEM                  = 102,
} CMErrors;

uint32_t cm_get_lib_error();

static inline uint32_t cm_get_os_error()
{
    return (uint32_t)syscall (OSIF_SYSCALL_GET_OS_ERROR, 0, 0, 0, 0, 0);
}
