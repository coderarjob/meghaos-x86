/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management Header
* ---------------------------------------------------------------------------
*/

#ifndef PMM_H_X86
#define PMM_H_X86

#include <stddef.h>
#include <stdbool.h>
#include <types.h>
#include <utils.h>
#include <config.h>
#include <buildcheck.h>

extern Physical g_pab; /* Address of Page Allocation Bitmap array */

typedef enum KernelPhysicalMemoryRegions
{
    PMM_REGION_ANY
} KernelPhysicalMemoryRegions;

typedef enum KernelPhysicalMemoryStates {
    PMM_STATE_FREE = 0,
    PMM_STATE_USED,
    PMM_STATE_RESERVED,
    PMM_STATE_INVALID
} KernelPhysicalMemoryStates;

// Calculates number of bytes from page frame count 'fc'.
#define PAGEFRAMES_TO_BYTES(fc) ((fc) * (CONFIG_PAGE_FRAME_SIZE_BYTES))

/** Number of complete page frames from at-most 'b' number of bytes. */
#define BYTES_TO_PAGEFRAMES_FLOOR(b)  ((b) / (CONFIG_PAGE_FRAME_SIZE_BYTES))

/** Number of complete page frames from at-least 'b' number of bytes. */
#define BYTES_TO_PAGEFRAMES_CEILING(b)  \
    (ALIGN_UP ((b), CONFIG_PAGE_FRAME_SIZE_BYTES) / CONFIG_PAGE_FRAME_SIZE_BYTES)

void kpmm_init ();
void kpmm_arch_init (U8 *pab);
bool kpmm_isInitialized ();

bool kpmm_free (Physical startAddress, UINT pageCount);
Physical kpmm_alloc (UINT pageCount, KernelPhysicalMemoryRegions reg);
bool kpmm_allocAt (Physical start, UINT pageCount, KernelPhysicalMemoryRegions reg);

size_t kpmm_getFreeMemorySize ();
U64 kpmm_arch_getInstalledMemoryByteCount ();
USYSINT kpmm_getUsableMemorySize (KernelPhysicalMemoryRegions reg);
#endif // PMM_H_X86
