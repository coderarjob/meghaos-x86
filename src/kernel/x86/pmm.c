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

static void s_markFreeMemory ();

/***************************************************************************************************
 * Initializes PAB array for x86 architecture.
 *
 * @input   pab      Pointer to arch independent PAB.
 * @return  nothing
 * @error   Panics if called after initialization.
 **************************************************************************************************/
void kpmm_arch_init (U8 *pab)
{
    (void)pab;
    s_markFreeMemory (pab);
}

/***************************************************************************************************
 * Marks pages which are free.
 *
 * It consults the bootloader structures and it marks free memory.
 * If memory map length is not aligned, memory is marked free, till the previous page boundary.
 * This means length is aligned to the previous multiple of CONFIG_PAGE_FRAME_SIZE_BYTES.
 *
 * @return  nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static void s_markFreeMemory ()
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
        // range. Now this is not an error, because on systems with more RAM than
        // MAX_PAB_ADDRESSABLE_BYTE, Memory map from BIOS will have sections that is entirely outside
        // the addressable range.
        if (lengthBytes == 0 || startAddress >= actualAddressableMemorySize) continue;

        // Check if addressing more than Addressable. Cap it to Max Addressable if so.
        U64 endAddress = startAddress + lengthBytes - 1;
        if (endAddress >= actualAddressableMemorySize)
            lengthBytes = actualAddressableMemorySize - startAddress + 1;

        // As access to this page is not allowed, if the start address is within the 1st page, we
        // need to move start location to the start of 2nd page frame and decrease the length by the
        // same amount.
        if (startAddress < CONFIG_PAGE_FRAME_SIZE_BYTES)
        {
            U64 distance = CONFIG_PAGE_FRAME_SIZE_BYTES - startAddress; // bytes till 2nd page.

            // Section starts within the 1st page, and end within the 1st page as well.
            // Thus we skip this section.
            if (lengthBytes <= distance) continue;

            // Section starts within the 1st page, but goes beyond the 1st page.
            lengthBytes -= distance;                        // adjust length
            startAddress = CONFIG_PAGE_FRAME_SIZE_BYTES;    // Start allocation from 2nd page
        }

        // If section length is less than CONFIG_PAGE_FRAME_SIZE_BYTES (4096 bytes), it cannot be
        // freed, so we skip this section
        if (lengthBytes < CONFIG_PAGE_FRAME_SIZE_BYTES)
            continue;

        // At this point, start and end address is within the addressable range.

        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_FLOOR ((USYSINT)lengthBytes);

        kdebug_printf ("\r\nI: Freeing startAddress: %llx, byteCount: %llx, pageFrames: %u."
                        , startAddress, lengthBytes, pageFrameCount);
        if (kpmm_free (createPhysical ((USYSINT)startAddress), pageFrameCount) == false)
            k_assertOnError ();
    }
}

/***************************************************************************************************
 * Gets the total amount of installed memory.
 *
 * Installed RAM size. OS may not be able to use the entire RAM.
 *
 * @return          Amount of installed RAM size in bytes.
 **************************************************************************************************/
U64 kpmm_arch_getInstalledMemoryByteCount ()
{
    BootLoaderInfo *bootLoaderInfo = kboot_getCurrentBootLoaderInfo ();
    U64 RAMSizeBytes = kboot_calculateAvailableMemory (bootLoaderInfo);

    return RAMSizeBytes;
}
