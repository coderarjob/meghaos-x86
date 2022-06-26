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

    // Calculates number of bytes from page frame count 'fc'.
    #define PAGEFRAMES_TO_BYTES(fc) (fc) * (CONFIG_PAGE_FRAME_SIZE_BYTES)

    // Calculates number of page frames from byte count 'b'.
    #define BYTES_TO_PAGEFRAMES(b)  (b) / (CONFIG_PAGE_FRAME_SIZE_BYTES)

    #define PAB_SIZE_BYTES                  CONFIG_PAGE_FRAME_SIZE_BYTES
    #define MAX_ADDRESSABLE_BYTE            (PAGEFRAMES_TO_BYTES(PAB_SIZE_BYTES * 8) - 1)
    #define MAX_ADDRESSABLE_BYTE_COUNT      MAX_ADDRESSABLE_BYTE + 1
    #define MAX_ADDRESSABLE_PAGE            ((PAB_SIZE_BYTES * 8) - 1)

#endif // __MOS_LIMITS_H_X86__
