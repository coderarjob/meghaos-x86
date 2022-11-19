/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*  TODO: Need to check if use of 'last byte/page' and 'byte/page' count concepts are used correctly.
*  TODO: Make allocation and deallocation atomic.
*/

#include <kernel.h>

static bool s_isPagesFree (UINT startPageFrame, UINT count, bool isDMA);
static void s_managePages (UINT startPageFrame, UINT frameCount, bool allocate, bool isDMA);
static INT s_get (UINT pageFrame, bool isDMA);
static void s_set (UINT pageFrame, bool alloc, bool isDMA);
static UINT s_getMaxPageCount (bool isDMA);
static void s_markMemoryOccupiedByModuleFiles ();
static void s_markFreeMemory ();

static U8 *s_pab = NULL;

/***************************************************************************************************
 * Initializes PAB array.
 *
 * It consults the bootloader structures and it marks free pages. These pages are not already being
 * used by the BIOS or APIC etc and are free to be used by the OS.
 *
 * @return nothing
 **************************************************************************************************/
void kpmm_init ()
{
    s_pab = (U8 *)CAST_PA_TO_VA (g_pab);
    k_memset (s_pab, 0xFF, PAB_SIZE_BYTES);

    // Mark free memory.
    s_markFreeMemory ();

    // Mark memory already occupied by the modules.
    s_markMemoryOccupiedByModuleFiles();
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
    for (INT i = 0; i < mmapCount; i++)
    {
        BootMemoryMapItem* memmap = kBootLoaderInfo_getMemoryMapItem (bootloaderinfo, i);
        BootMemoryMapTypes type = kBootMemoryMapItem_getType (memmap);

        if (type != MMTYPE_FREE) continue;

        USYSINT startAddress = (USYSINT)kBootMemoryMapItem_getBaseAddress (memmap);
        USYSINT lengthBytes = (USYSINT)kBootMemoryMapItem_getLength (memmap);

        // Handle the case where the start address is within the 1st page. As access to this page is
        // not allowed, we need to move start location to the start of 2nd page frame and adjust the
        // length, so that the end address of this block remains the same.
        if (startAddress < CONFIG_PAGE_FRAME_SIZE_BYTES)
        {
            lengthBytes -= (CONFIG_PAGE_FRAME_SIZE_BYTES - startAddress); // adjust length
            startAddress = CONFIG_PAGE_FRAME_SIZE_BYTES;    // Start allocation from 2nd page
        }

        // Check if addressing more than Addressable. Cap it to Max Addressable if so.
        ULLONG endAddress = startAddress + lengthBytes - 1;
        endAddress = (endAddress < MAX_ADDRESSABLE_BYTE_COUNT) ? endAddress
                                                               : MAX_ADDRESSABLE_BYTE;

        // Actual number of bytes we can free without crossing the max addressable range.
        USYSINT lengthBytes_possible = (USYSINT)(endAddress - startAddress  + 1);
        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_FLOOR (lengthBytes_possible);

        kdebug_printf ("\r\nI: Freeing startAddress: %px, byteCount: %px, pageFrames: %u."
                        , startAddress, lengthBytes, pageFrameCount);
        if (kpmm_free (createPhysical (startAddress), pageFrameCount) == false)
            k_assertOnError ();
    }
}

/***************************************************************************************************
 * Marks pages occupied by module files as occupied.
 *
 * It consults the bootloader structures and it marks memory occupied by module files.
 * If memory map length is not aligned, memory is marked allocated, till the next page boundary.
 * This means length is aligned to the next multiple of CONFIG_PAGE_FRAME_SIZE_BYTES.
 *
 * @return nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static void s_markMemoryOccupiedByModuleFiles ()
{
    BootLoaderInfo *bootloaderinfo = kboot_getCurrentBootLoaderInfo ();
    INT filesCount = kBootLoaderInfo_getFilesCount (bootloaderinfo);
    for (INT i = 0; i < filesCount; i++)
    {
        BootFileItem* fileinfo = kBootLoaderInfo_getFileItem (bootloaderinfo, i);
        USYSINT startAddress = (USYSINT)kBootFileItem_getStartLocation (fileinfo);
        USYSINT lengthBytes = (USYSINT)kBootFileItem_getLength (fileinfo);
        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_CEILING (lengthBytes);

        kdebug_printf ("\r\nI: Allocate startAddress: %px, byteCount: %px, pageFrames: %u."
                        , startAddress, lengthBytes, pageFrameCount);
        if (kpmm_allocAt (createPhysical(startAddress), pageFrameCount, FALSE) == false)
            k_assertOnError ();
    }
}
/***************************************************************************************************
 * Deallocates specified pages starting from the specified physical location.
 *
 * @Input startAddress  Physical memory location of the first page. Error is generated if not a
 *                      aligned to page boundary.
 * @Input pageCount     Number of page frames to deallocate.
 * @return              If successful, returns true;
 * @return              If failure false is returned. k_errorNumber is set with error code.
 *                      1. ERR_WRONG_ALIGNMENT  - startAddress not aligned to page boundary.
 *                      2. ERR_INVALID_ARGUMENT - pageCount is zero.
 *                      3. ERR_OUTSIDE_ADDRESSABLE_RANGE - the provided address is more than the max
 *                                                         range.
 **************************************************************************************************/
bool kpmm_free (PHYSICAL startAddress, UINT pageCount)
{
    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    // Check if address is within the max addressable range.
    USYSINT allocation_end_byte = startAddress.val + (pageCount * CONFIG_PAGE_FRAME_SIZE_BYTES) - 1;

    if (allocation_end_byte >= MAX_ADDRESSABLE_BYTE_COUNT)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, false);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (startAddress.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    // Note: As startAddress is already aligned, both floor or ceiling are same here.
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (startAddress.val);

    // DMA addressing range does not determine freeing. FALSE if passed here becuuse we want free to
    // deallocate any address.
    s_managePages (startPageFrame, pageCount, false, FALSE);

    return true;
}

/***************************************************************************************************
 * Tries to allocates pages starting at the physical address that is provided.
 *
 * @Input pageCount     Number of byte frames to allocate.
 * @Input isDMA         Is allocating from the DMA memory range.
 * @Input start         Pages will be allocated from this physical address.Must be page aligned.
 * @return              If successful, returns true.
 * @return              If failure false is returned. k_errorNumber is set with error code.
 *                      1. ERR_WRONG_ALIGNMENT - 'start' is not aligned to page boundary.
 *                      2. ERR_DOUBLE_ALLOC - All or part of specified memory is already allocated.
 *                                            This error is only thrown for FIXED allocations.
 *                      3. ERR_INVALID_ARGUMENT - pageCount is zero.
 *                      4. ERR_OUTSIDE_ADDRESSABLE_RANGE - the provided address is more than the max
 *                                                         range.
 **************************************************************************************************/
bool kpmm_allocAt (PHYSICAL start, UINT pageCount, bool isDMA)
{
    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    // Check if address is within the max addressable range.
    USYSINT max_addressable_byte_count = s_getMaxPageCount (isDMA) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    USYSINT allocation_end_byte = start.val + (pageCount * CONFIG_PAGE_FRAME_SIZE_BYTES) - 1;
    if (allocation_end_byte >= max_addressable_byte_count)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, false);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (start.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    // Note: Because the address is aligned to page boundary, Floor or ceiling are the same.
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (start.val);

    // Check if all the pages can be allocated at the provided location.
    bool found = s_isPagesFree (startPageFrame, pageCount, isDMA);

    // Free pages were not found. But there was no error.
    if (found == FALSE)
        RETURN_ERROR (ERR_DOUBLE_ALLOC, false);

    // Free pages found. Now Allocate them.
    s_managePages (startPageFrame, pageCount, true, isDMA);

    return true;
}

/***************************************************************************************************
 * Tries to allocates pages starting at the physical address that is provided.
 *
 * @Input pageCount     Number of byte frames to allocate.
 * @Input isDMA         Is allocating from the DMA memory range.
 * @return              If successful, returns the allocated physical address.
 * @return              If failure PHYSICAL_NULL is returned. k_errorNumber is set with error code.
 *                      1. ERR_OUT_OF_MEM   - Could not find the required number of free
 *                                            consecutive pages.
 *                      2. ERR_INVALID_ARGUMENT - pageCount is zero.
 **************************************************************************************************/
PHYSICAL kpmm_alloc (UINT pageCount, bool isDMA)
{
    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, PHYSICAL_NULL);

    UINT startPageFrame = 1;
    INT found = FALSE;

    // Search PAB for a suitable location.
    UINT maxPageFrame = s_getMaxPageCount (isDMA) - pageCount;
    for (; found == FALSE && startPageFrame <= maxPageFrame; startPageFrame++)
        found = s_isPagesFree (startPageFrame, pageCount, isDMA);

    --startPageFrame; // undoing the last increment.

    // Free pages were not found. But there was no error.
    if (found == FALSE)
        RETURN_ERROR (ERR_OUT_OF_MEM, PHYSICAL_NULL);

    // Free pages found. Now Allocate them.
    if (found == TRUE)
    {
        s_managePages (startPageFrame, pageCount, true, isDMA);
        return createPhysical (PAGEFRAMES_TO_BYTES (startPageFrame));
    }

    // There was an error in either s_isPagesFree or s_managePages
    return PHYSICAL_NULL;
}

/***************************************************************************************************
 * Marks physical memory pages as either Allocated or Free in PAB.
 *
 * 'frameCount' pages of memory starting from 'startPageFrame' will be either marked as Allocated
 * or Free.
 *
 * @Input startPageFrame    Physical page where the allocation must begin. First page frame is 0.
 * @Input frameCount        Number of page frames to allocate. Out of Memory is generated if more
 *                          pages are requested than are free.
 * @Input allocate          true to Allocate, false to deallocate.
 * @return                  On failure panic is triggered.
 **************************************************************************************************/
static void s_managePages (UINT startPageFrame, UINT frameCount, bool allocate, bool isDMA)
{
    kdebug_printf ("\r\nI: %s 0x%px bytes starting physical address 0x%px."
                    , (allocate) ? "Allocating" : "Freeing"
                    , PAGEFRAMES_TO_BYTES(frameCount), PAGEFRAMES_TO_BYTES(startPageFrame));

    k_assert (frameCount > 0, "Page frame count cannot be zero.");

    for (UINT pageFrame = startPageFrame ; frameCount-- > 0; pageFrame++)
        s_set (pageFrame, allocate, isDMA);
}

/***************************************************************************************************
 * Checks if 'count' number of consecutive frames are free.
 *
 * @Input startPageFrame    Physical page frame to start the check from. First page frame is 0.
 * @Input count             Number of consecutive page frames that need to be free.
 * @return                  Returns TRUE if required number of free pages were found at the physical
 *                          location.
 * @return                  Returns FALSE if required number of free pages were not found at the
 *                          physical location.
 * @return                  On failure panic is triggered.
 **************************************************************************************************/
static bool s_isPagesFree (UINT startPageFrame, UINT count, bool isDMA)
{
    k_assert (count > 0, "Page frame count cannot be zero.");

    INT isAllocated = 0;
    for (UINT i = 0 ; i < count && isAllocated == 0; i++)
        isAllocated = s_get (i + startPageFrame, isDMA);

    return isAllocated == 0;
}

/***************************************************************************************************
 * Sets/Clears corresponding page frame bit in PAB.
 *
 * @Input pageFrame Physical page frame to change. First page frame is 0.
 * @return          If failure panic is triggered.
 **************************************************************************************************/
static void s_set (UINT pageFrame, bool alloc, bool isDMA)
{
    if (pageFrame == 0)
        k_panic ("Invalid access: Page %u", pageFrame);

    INT allocated = s_get (pageFrame, isDMA);

    if (allocated && alloc)
        k_panic ("Double allocation: Page %u", pageFrame);

    if (!allocated && !alloc)
        k_panic ("Double free: Page %u", pageFrame);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
    UINT mask = (U8)(1U << bitIndex);

    if (alloc)
        s_pab[byteIndex] |= (U8)mask;
    else
        s_pab[byteIndex] &= (U8)~mask;
}

/***************************************************************************************************
 * Gets corresponding page frame bit from PAB.
 *
 * @Input pageFrame Physical page frame to query. First page frame is 0.
 * @return          On Success, returns 0 is page frame is free, 1 otherwise.
 * @return          If failure panic is triggered.
 **************************************************************************************************/
static INT s_get (UINT pageFrame, bool isDMA)
{
    if (pageFrame == 0)
        k_panic ("Invalid access: Page %u", pageFrame);

    if (pageFrame > s_getMaxPageCount (isDMA))
        k_panic ("Access outside range. Page %u", pageFrame);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
    UINT mask = (U8)(1U << bitIndex);

    return (s_pab[byteIndex] & (U8)mask) >> bitIndex;
}

/***************************************************************************************************
 * Returs the largest page frame count, for the input provided.
 *
 * @Input isDMA     Queried for DMA.
 * @return          Gets the maximum page frame count for DMA hardware. Otherwise returns the
 *                  maximum page frame count supported by the PAB.
 **************************************************************************************************/
static UINT s_getMaxPageCount (bool isDMA)
{
    UINT maxPageCount = (isDMA) ? MAX_DMA_ADDRESSABLE_PAGE_COUNT: MAX_ADDRESSABLE_PAGE_COUNT;
    k_assert (maxPageCount <= MAX_ADDRESSABLE_PAGE_COUNT, "Page outside addressable range");

    return maxPageCount;
}
