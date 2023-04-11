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

#ifndef MOS_LIMITS_H_X86
#define MOS_LIMITS_H_X86

#include <config.h>
#include <buildcheck.h>

#define MAX_VGA_COLUMNS                    80U
#define MAX_VGA_ROWS                       50U

#define MAX_PRINTABLE_STRING_LENGTH        MAX_VGA_COLUMNS * MAX_VGA_ROWS

/* Minimum index that can be editted in Kernel. */
#define MIN_GDT_INDEX                      3U

/* Number of GDT entries in memory */
#define MAX_GDT_DESC_COUNT                 512U

/* Maximum number of IDT entries */
#define MAX_IDT_DESC_COUNT                 256U

/* DMA Limit */
#define MAX_DMA_BYTE_COUNT                 (16U * MB)
#define MAX_DMA_PAGE_COUNT                 (BYTES_TO_PAGEFRAMES_FLOOR(MAX_DMA_BYTE_COUNT))

/* PAB and addressable RAM */
#define PAB_SIZE_BYTES                     CONFIG_PAGE_FRAME_SIZE_BYTES
#define MAX_PAB_ADDRESSABLE_PAGE_COUNT     (PAB_SIZE_BYTES * 8)
#define MAX_PAB_ADDRESSABLE_BYTE_COUNT     (PAGEFRAMES_TO_BYTES(MAX_PAB_ADDRESSABLE_PAGE_COUNT))
#define MAX_PAB_DMA_ADDRESSABLE_PAGE_COUNT (MIN (MAX_PAB_ADDRESSABLE_PAGE_COUNT,MAX_DMA_PAGE_COUNT))
#define MAX_PAB_DMA_ADDRESSABLE_BYTE_COUNT (MIN (MAX_PAB_ADDRESSABLE_BYTE_COUNT,MAX_DMA_BYTE_COUNT))

#define MAX_PAB_ADDRESSABLE_BYTE            (MAX_PAB_ADDRESSABLE_BYTE_COUNT - 1)
#define MAX_PAB_ADDRESSABLE_PAGE            (MAX_PAB_ADDRESSABLE_PAGE_COUNT - 1)
#define MAX_PAB_DMA_ADDRESSABLE_BYTE        (MAX_PAB_DMA_ADDRESSABLE_BYTE_COUNT - 1)
#define MAX_PAB_DMA_ADDRESSABLE_PAGE        (MAX_PAB_DMA_ADDRESSABLE_PAGE_COUNT - 1)

// Interrupt/Exception frame size
#define INTERRUPT_FRAME_SIZE_BYTES          (4 * 12)

#endif // MOS_LIMITS_H_X86
