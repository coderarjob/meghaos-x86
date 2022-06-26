/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*  TODO: Functions should not throw asserts, they must set k_errorNumber and return.
*  TODO: Need to check if use of 'last byte/page' and 'byte/page' count concepts are used correctly.
*  TODO: Should responsibility of aligning bytesCount lie with the caller??
*/

#include <kernel.h>

static void s_allocatedPage (UINT pageFrame);
static void s_freePage (UINT pageFrame);
static bool s_isPageAllocated (UINT pageFrame);
static bool s_isPagesFree (UINT startPageFrame, UINT count);
static void s_allocateDeallocatePages (UINT startPageFrame, UINT frameCount, bool allocate);
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
    UINT mmapCount = kboot_getBootLoaderInfoBootMemoryMapItemCount (bootloaderinfo);
    for (int i = 0; i < mmapCount; i++)
    {
        BootMemoryMapItem* memmap = kboot_getBootLoaderInfoBootMemoryMapItem (bootloaderinfo, i);
        BootMemoryMapTypes type = kboot_getBootMemoryMapItemType (memmap);
        USYSINT startAddress = (USYSINT)kboot_getBootMemoryMapItemBaseAddress (memmap);
        USYSINT lengthBytes = (USYSINT)kboot_getBootMemoryMapItemLengthBytes (memmap);

        if (type == MMTYPE_FREE)
        {
            kdebug_printf ("\r\nI: startAddress: %px, byteCount: %px.", startAddress, lengthBytes);
            kpmm_free (startAddress, ALIGN_DOWN (lengthBytes, CONFIG_PAGE_FRAME_SIZE_BYTES));
        }
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
    UINT filesCount = kboot_getBootLoaderInfoFilesCount (bootloaderinfo);
    for (int i = 0; i < filesCount; i++)
    {
        BootFileItem* fileinfo = kboot_getBootLoaderInfoBootFileItem (bootloaderinfo, i);
        USYSINT startAddress = (USYSINT)kboot_getBootFileItemStartLocation (fileinfo);
        USYSINT lengthBytes = (USYSINT)kboot_getBootFileItemFileLength (fileinfo);

        kdebug_printf ("\r\nI: startAddress: %px, byteCount: %px.", startAddress, lengthBytes);
        kpmm_alloc (NULL , &lengthBytes , PMM_FIXED , startAddress);
    }
}
/***************************************************************************************************
 * Deallocates specified pages starting from the specified physical location.
 *
 * @param startAddress  Physical memory location of the first page. Assert is thrown if not aligned 
 *                      to page boundary.
 * @param byteCount     Number of bytes to deallocate. Assert is thrown if not aligned to page
 *                      boundary.
 * @return true         No error. Deallocation was successful.
 * @return false        Deallocation was not successful. k_errorNumber is set.
 **************************************************************************************************/
bool kpmm_free (PHYSICAL startAddress, USYSINT byteCount)
{
    k_assert (IS_ALIGNED (byteCount, CONFIG_PAGE_FRAME_SIZE_BYTES), "Count is not aligned");
    k_assert (IS_ALIGNED (startAddress.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address is not aligned");

    UINT startPageFrame = BYTES_TO_PAGEFRAMES (startAddress.val);
    UINT pageFrameCount = BYTES_TO_PAGEFRAMES (byteCount);
    s_allocateDeallocatePages (startPageFrame, pageFrameCount, false);
    return true;
}

/***************************************************************************************************
 * Searches the PAB and allocated the specified number of consecutive free physical pages.
 *
 * @param allocated     If successful, holds the allocated physical address. If type is PMM_FIXED,
 *                      this can be NULL.
 * @param byteCount     Number of bytes to allocate. Will be rounded to the next nearest multiple
 *                      of CONFIG_PAGE_FRAME_SIZE_BYTES. The variable is modified to the aligned 
 *                      byteCount.
 * @param type          PMM_AUTOMATIC to automatically find a suitable physical address.
 * @param type          PMM_FXED to allocate the specified number of bytes at the specified address.
 * @param start         Physical address where allocation will be attempted if type is PMM_FIXED.
 *                      Omitted otherwise.
 * @return true         No error. Allocation was successful.
 * @return false        Out of Memory. Allocation was not successful. k_errorNumber is set.
 **************************************************************************************************/
bool kpmm_alloc (PHYSICAL *allocated, USYSINT *byteCount, PMMAllocationTypes type, PHYSICAL start)
{
    UINT startPageFrame = 0;
    bool found = false;

    // Make sure multiple of CONFIG_PAGE_FRAME_SIZE_BYTES is requested.
    *byteCount = (*byteCount > MAX_ADDRESSABLE_BYTE_COUNT)
                 ? MAX_ADDRESSABLE_BYTE_COUNT
                 : ALIGN_UP (*byteCount, CONFIG_PAGE_FRAME_SIZE_BYTES);

    // Find enough consecutive free physical pages or check if enough consecutive free pages exist
    // at the provided address.
    UINT pageFrameCount = BYTES_TO_PAGEFRAMES (*byteCount);

    switch (type)
    {
    case PMM_AUTOMATIC:
        for ( ; found == false && startPageFrame < MAX_ADDRESSABLE_PAGE ; startPageFrame++)
            found = s_isPagesFree (startPageFrame, pageFrameCount);
        break;
    case PMM_FIXED:
        k_assert (IS_ALIGNED (start.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address is not aligned");
        startPageFrame = BYTES_TO_PAGEFRAMES (start.val);
        found = s_isPagesFree (startPageFrame, pageFrameCount);
        break;
    default:
        // Should not be here.
        k_assert (false, "Should not be here. Invalid Allocation Type");
    }

    // Not enough free pages found.
    if (found == false) goto notfound;

    // Free pages found. Now Allocate them.
    USYSINT startAddress = PAGEFRAMES_TO_BYTES (startPageFrame);
    s_allocateDeallocatePages (startPageFrame, pageFrameCount, true);

    if ((type == PMM_AUTOMATIC || (type == PMM_FIXED && allocated != NULL)))
        (*allocated).val = startAddress;

    RETURN_ERROR (ERR_NONE, true);
notfound:
    RETURN_ERROR (ERR_PMM_OUT_OF_MEM, false);
}

/***************************************************************************************************
 * Marks physical memory pages as either Allocated or Free in PAB.
 *
 * 'frameCount' pages of memory starting from 'startPageFrame' will be either marked as Allocated
 * or Free.
 *
 * @param startPageFrame    Physical page where the allocation must begin. Index starts from 0.
 * @param frameCount        Number of page frames to allocate. Out of Memory is generated if more
 *                          pages are requested than are free.
 * @param allocate          true to Allocate, false to deallocate.
 * @return                  nothing
 **************************************************************************************************/
static void s_allocateDeallocatePages (UINT startPageFrame, UINT frameCount, bool allocate)
{
    UINT endPageFrame = startPageFrame + frameCount - 1;

    kdebug_printf ("\r\n%s 0x%px bytes starting physical address 0x%px."
                    , (allocate) ? "Allocating" : "Freeing"
                    , PAGEFRAMES_TO_BYTES(frameCount), PAGEFRAMES_TO_BYTES(startPageFrame));

    for (UINT pageFrame = startPageFrame
            ; pageFrame <= endPageFrame
            ; pageFrame++)
    {
        (allocate) ? s_allocatedPage (pageFrame)
                   : s_freePage (pageFrame);
    }
}

/***************************************************************************************************
 * Checks if 'count' number of consecutive frames are free.
 *
 * @param startPageFrame    Physical page frame to start the check from.
 * @param count             Number of consecutive page frames that need to be free.
 * @return true             if required number of free pages is found at the physical location.
 * @return false            if required number of free pages is not found at the physical location.
 **************************************************************************************************/
static bool s_isPagesFree (UINT startPageFrame, UINT count)
{
    bool isFree = true;
    for (UINT i = 0 ; i < count && isFree; i++)
        isFree = !s_isPageAllocated (i + startPageFrame);

    return isFree;
}

/***************************************************************************************************
 * Checks if a physical page frame is allocated.
 *
 * @param pageFrame Physical page frame to check. A warning is generated if pageFrame is larger than
 *                  the maximum addressable page frame.
 * @return true     if the page frame is allocated.
 * @return false    if the page frame is free.
 **************************************************************************************************/
static bool s_isPageAllocated (UINT pageFrame)
{
    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);

    // Any page beyond what can be addressed via the PAB is treated as not free.
    if (byteIndex >= PAB_SIZE_BYTES)
    {
        kdebug_printf ("\r\nW: Trying to access inaccessible frame: %u", pageFrame);
        return true;
    }

    return s_pab[byteIndex] & (U8)(1U << bitIndex);
}

/***************************************************************************************************
 * Marks a physical page frame as free.
 *
 * Asserts is page frame is already free.
 *
 * @param pageFrame Physical page frame to check.
 * @return nothing.
 **************************************************************************************************/
static void s_freePage (UINT pageFrame)
{
    k_assert (s_isPageAllocated (pageFrame), "Double free");

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
    k_assert (byteIndex < PAB_SIZE_BYTES, "PAB index overflow");

    s_pab[byteIndex] &= (U8)(~(1U << bitIndex));
}

/***************************************************************************************************
 * Marks a physical page frame as allocated.
 *
 * Asserts is page frame is already allocated.
 *
 * @param pageFrame Physical page frame to check.
 * @return nothing.
 **************************************************************************************************/
static void s_allocatedPage (UINT pageFrame)
{
    k_assert (!s_isPageAllocated (pageFrame), "Double allocation");

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);

    k_assert (byteIndex < PAB_SIZE_BYTES, "PAB index overflow");

    s_pab[byteIndex] |= (U8)(1U << bitIndex);
}
