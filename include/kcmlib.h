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

typedef struct CM_MallocHeader
{
    size_t netNodeSize; /// Size of a region together with the header size.
    bool isAllocated;   /// Is the region allocated or free.
    ListNode adjnode;   /// A node in the Adjacent list.
    ListNode freenode;  /// A node in the Free list.
    ListNode allocnode; /// A node in the Allocation list
} CM_MallocHeader;

#define CM_MALLOC_MEM_SIZE_BYTES (ARCH_MEM_LEN_BYTES_PROCESS_DATA / 2)

extern uint32_t cm_error_num;

/* Can be used to store an error code and return from a function */
#define CM_RETURN_ERROR(errno, rval)       \
    do {                                   \
        CM_DBG_ERROR ("Error %x.", errno); \
        cm_error_num = errno;              \
        return rval;                       \
    } while (0)
