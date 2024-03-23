/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*  TODO: Need to check if use of 'last byte/page' and 'byte/page' count concepts are used correctly.
*  TODO: Make allocation and deallocation atomic.
*/

#include <pmm.h>
#include <moslimits.h>
#include <types.h>
#include <x86/boot.h>
#include <kerror.h>
#include <panic.h>
#include <paging.h>
#include <kdebug.h>
#include <kstdlib.h>

static void s_markFreeMemory (Bitmap *bitmap);

/***************************************************************************************************
 * Initializes PAB array for x86 architecture.
 * All physical memory that is not Free (0) or Used (1) will be marked Reserved (2).
 *
 * @input   pab      Pointer to arch independent PAB.
 * @return  nothing
 **************************************************************************************************/
void kpmm_arch_init (Bitmap *bitmap)
{
    FUNC_ENTRY();

    k_memset (bitmap->bitmap, 0xAA, PAB_SIZE_BYTES);
    s_markFreeMemory (bitmap);
}

/***************************************************************************************************
 * Consults the bootloader structures and it marks free pages.
 * If length of a memory map is not aligned, memory till the last page boundary is marked.
 *
 * @return  nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static void s_markFreeMemory (Bitmap *bitmap)
{
    BootLoaderInfo *bootloaderinfo = kboot_getCurrentBootLoaderInfo ();
    INT mmapCount = kBootLoaderInfo_getMemoryMapItemCount (bootloaderinfo);
    size_t actualAddressableMemorySize = kpmm_getUsableMemorySize (PMM_REGION_ANY);

    for (INT i = 0; i < mmapCount; i++)
    {
        BootMemoryMapItem* memmap = kBootLoaderInfo_getMemoryMapItem (bootloaderinfo, i);
        BootMemoryMapTypes type = kBootMemoryMapItem_getType (memmap);

        if (type != MMTYPE_FREE) continue;

        U64 startAddress = kBootMemoryMapItem_getBaseAddress (memmap);
        U64 lengthBytes = kBootMemoryMapItem_getLength (memmap);

        // Skip, if length is zero or start address of the section is beyond the addressable
        // range (systems can have more RAM installed than our PAB supports).
        if (lengthBytes == 0 || startAddress >= actualAddressableMemorySize) continue;

        // Check if addressing more than Addressable. Cap it to Max Addressable if so.
        U64 endAddress = MIN(actualAddressableMemorySize - 1, startAddress + lengthBytes - 1);
        lengthBytes = endAddress - startAddress + 1;

        // If section length is less than 1 page size  (4096 bytes), we free the whole page.
        lengthBytes = MAX(CONFIG_PAGE_FRAME_SIZE_BYTES, lengthBytes);

        // At this point, start and end address is within the addressable range.
        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_FLOOR ((USYSINT)lengthBytes);
        UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR ((USYSINT)startAddress);

        bool success = bitmap_setContinous(bitmap, startPageFrame, pageFrameCount,
                                           PMM_STATE_FREE);
        if (!success)
            k_panic("PAB cannot be initialized.");
    }
}

/***************************************************************************************************
 * Gets the total amount of installed memory.
 * This is the size of installed RAM, which may be is different than the usable size of the RAM. The
 * Usable size is determined by the PAB size.
 *
 * @return          Amount of installed RAM size in bytes.
 **************************************************************************************************/
U64 kpmm_arch_getInstalledMemoryByteCount ()
{
    FUNC_ENTRY();

    BootLoaderInfo *bootLoaderInfo = kboot_getCurrentBootLoaderInfo ();
    U64 RAMSizeBytes = kboot_calculateAvailableMemory (bootLoaderInfo);
    INFO ("RAMSizeBytes = 0x%llx", RAMSizeBytes);

    return RAMSizeBytes;
}
