/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - App library Kernel Header - Private type/function declarations
 * ---------------------------------------------------------------------------
 * Types and function declarations defined here are private to the CM library and should not be
 * exposed to the applications.
 */

#include <intrusive_list.h>
#include <memloc.h>

#pragma once

extern uint32_t cm_error_num;

/* Can be used to store an error code and return from a function */
#define CM_RETURN_ERROR(errno, rval)       \
    do {                                   \
        CM_DBG_ERROR ("Error %x.", errno); \
        cm_error_num = errno;              \
        return rval;                       \
    } while (0)
