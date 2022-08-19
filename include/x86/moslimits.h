/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Limits
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 7th September 2021
*/

#ifndef __MOS_LIMITS_H_X86__
#define __MOS_LIMITS_H_X86__
#include <kernel.h>

#define MAX_VGA_COLUMNS                 80
#define MAX_VGA_ROWS                    50

#define MAX_PRINTABLE_STRING_LENGTH     MAX_VGA_COLUMNS * MAX_VGA_ROWS

/* Minimum index that can be editted in Kernel. */
#define MIN_GDT_INDEX                   3

/* Number of GDT entries in memory */
#define MAX_GDT_DESC_COUNT              512

/* Maximum number of IDT entries */
#define MAX_IDT_DESC_COUNT              256

/* PAB and addressable RAM */
#define PAB_SIZE_BYTES                  CONFIG_PAGE_FRAME_SIZE_BYTES
#define MAX_ADDRESSABLE_BYTE_COUNT      (PAGEFRAMES_TO_BYTES(PAB_SIZE_BYTES * 8))
#define MAX_ADDRESSABLE_PAGE_COUNT      (PAB_SIZE_BYTES * 8)

#define MAX_ADDRESSABLE_BYTE            (MAX_ADDRESSABLE_BYTE_COUNT - 1)
#define MAX_ADDRESSABLE_PAGE            (MAX_ADDRESSABLE_PAGE_COUNT - 1)

/* DMA Limit */
#define MAX_DMA_BYTE_COUNT              (16 * MB)
#define MAX_DMA_PAGE_COUNT              (BYTES_TO_PAGEFRAMES_FLOOR(MAX_DMA_BYTE_COUNT))

#define MAX_DMA_ADDRESSABLE_PAGE_COUNT  (MIN (MAX_ADDRESSABLE_PAGE_COUNT, MAX_DMA_PAGE_COUNT))
#define MAX_DMA_ADDRESSABLE_BYTE_COUNT  (MIN (MAX_ADDRESSABLE_BYTE_COUNT, MAX_DMA_BYTE_COUNT))
#define MAX_DMA_ADDRESSABLE_BYTE        (MAX_DMA_ADDRESSABLE_BYTE_COUNT - 1)
#define MAX_DMA_ADDRESSABLE_PAGE        (MAX_DMA_ADDRESSABLE_PAGE_COUNT - 1)
#endif // __MOS_LIMITS_H_X86__
