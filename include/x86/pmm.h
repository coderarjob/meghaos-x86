/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management Header
* ---------------------------------------------------------------------------
*/

#ifndef PMM_H_X86
#define PMM_H_X86

#include <kernel.h>

extern PHYSICAL g_pab; /* Address of Page Allocation Bitmap array */

typedef enum PMMAllocationTypes
{
    PMM_AUTOMATIC,
    PMM_FIXED
} PMMAllocationTypes;

// Calculates number of bytes from page frame count 'fc'.
#define PAGEFRAMES_TO_BYTES(fc) (fc) * (CONFIG_PAGE_FRAME_SIZE_BYTES)

/** Number of complete page frames from at-most 'b' number of bytes. */
#define BYTES_TO_PAGEFRAMES_FLOOR(b)  (b) / (CONFIG_PAGE_FRAME_SIZE_BYTES)

/** Number of complete page frames from at-least 'b' number of bytes. */
#define BYTES_TO_PAGEFRAMES_CEILING(b)  \
    ALIGN_UP ((b), CONFIG_PAGE_FRAME_SIZE_BYTES) / (CONFIG_PAGE_FRAME_SIZE_BYTES)

void kpmm_init ();
INT kpmm_free (PHYSICAL startAddress, UINT pageCount);
INT kpmm_alloc (PHYSICAL *allocated, UINT pageCount, PMMAllocationTypes type, PHYSICAL start);
#endif // PMM_H_X86
