/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Physical Memory Management Header
* ---------------------------------------------------------------------------
*/

#ifndef PMM_H_X86
#define PMM_H_X86

#include <stddef.h>
#include <stdbool.h>
#include <types.h>
#include <utils.h>
#include <config.h>
#include <common/bitmap.h>

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

void kpmm_init ();
void kpmm_arch_init (Bitmap *bitmap);

bool kpmm_free (Physical startAddress, UINT pageCount);
bool kpmm_alloc (Physical *address, UINT pageCount, KernelPhysicalMemoryRegions reg);
bool kpmm_allocAt (Physical start, UINT pageCount, KernelPhysicalMemoryRegions reg);

size_t kpmm_getFreeMemorySize ();
U64 kpmm_arch_getInstalledMemoryByteCount ();
USYSINT kpmm_getUsableMemorySize (KernelPhysicalMemoryRegions reg);
KernelPhysicalMemoryStates kpmm_getPageStatus(Physical phy);
#endif // PMM_H_X86
