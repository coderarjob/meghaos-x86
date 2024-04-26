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

#include <buildcheck.h>
#include <config.h>
#include <paging.h>

#define MAX_VGA_COLUMNS             80U
#define MAX_VGA_ROWS                50U
#define MAX_PRINTABLE_STRING_LENGTH (MAX_VGA_COLUMNS * 3)

#define MIN_GDT_INDEX      3U   /* Minimum index that can be editted in Kernel. */
#define MAX_GDT_DESC_COUNT 512U /* Number of GDT entries in memory */
#define MAX_IDT_DESC_COUNT 256U /* Maximum number of IDT entries */

/* PAB and addressable RAM */
#define PAB_BITS_PER_STATE 2U
#define PAB_SIZE_BYTES     (CONFIG_PAGE_FRAME_SIZE_BYTES * CONFIG_PAB_NUMBER_OF_PAGES)

#define MAX_PAB_ADDRESSABLE_PAGE_COUNT (PAB_SIZE_BYTES * 8 / PAB_BITS_PER_STATE)
#define MAX_PAB_ADDRESSABLE_BYTE_COUNT (PAGEFRAME_TO_PHYSICAL (MAX_PAB_ADDRESSABLE_PAGE_COUNT))
#define MAX_PAB_ADDRESSABLE_BYTE       (MAX_PAB_ADDRESSABLE_BYTE_COUNT - 1)
#define MAX_PAB_ADDRESSABLE_PAGE       (MAX_PAB_ADDRESSABLE_PAGE_COUNT - 1)

// Interrupt/Exception frame size
#define INTERRUPT_FRAME_SIZE_BYTES (4 * 12)

#endif // MOS_LIMITS_H_X86
