/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*  TODO: Functions should not throw asserts, they must set k_errorNumber and return. [Done]
*  TODO: Need to check if use of 'last byte/page' and 'byte/page' count concepts are used correctly.
*  TODO: Should responsibility of aligning bytesCount lie with the caller?? [DONE]
*  TODO: CAST_PA_TO_VA inline function is not testable. Do something. Macro??
*  TODO: Consider the case where there is more RAM than the MAX supported by the PAB size. [Done]
*/

#include <kernel.h>

static INT s_allocatedPage (UINT pageFrame);
static INT s_freePage (UINT pageFrame);
static INT s_isPageAllocated (UINT pageFrame);
static INT s_isPagesFree (UINT startPageFrame, UINT count);
static INT s_allocateDeallocatePages (UINT startPageFrame, UINT frameCount, bool allocate);
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

        if (type != MMTYPE_FREE) continue;

        USYSINT startAddress = (USYSINT)kboot_getBootMemoryMapItemBaseAddress (memmap);
        USYSINT lengthBytes = (USYSINT)kboot_getBootMemoryMapItemLengthBytes (memmap);

        // Check if addressing more than Addressable. Cap it to Max Addressable if so.
        ULLONG endAddress = startAddress + lengthBytes - 1;
        endAddress = (endAddress < MAX_ADDRESSABLE_BYTE_COUNT) ? endAddress
                                                               : MAX_ADDRESSABLE_BYTE;

        // Actual number of bytes we can free without crossing the max addressable range.
        USYSINT lengthBytes_possible = endAddress - startAddress  + 1;
        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_FLOOR (lengthBytes_possible);

        kdebug_printf ("\r\nI: Freeing startAddress: %px, byteCount: %px, pageFrames: %u."
                        , startAddress, lengthBytes, pageFrameCount);
        if (kpmm_free (startAddress, pageFrameCount) == EXIT_FAILURE)
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
    UINT filesCount = kboot_getBootLoaderInfoFilesCount (bootloaderinfo);
    for (int i = 0; i < filesCount; i++)
    {
        BootFileItem* fileinfo = kboot_getBootLoaderInfoBootFileItem (bootloaderinfo, i);
        USYSINT startAddress = (USYSINT)kboot_getBootFileItemStartLocation (fileinfo);
        USYSINT lengthBytes = (USYSINT)kboot_getBootFileItemFileLength (fileinfo);
        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_CEILING (lengthBytes);

        kdebug_printf ("\r\nI: Allocate startAddress: %px, byteCount: %px, pageFrames: %u."
                        , startAddress, lengthBytes, pageFrameCount);
        if (kpmm_alloc (NULL , pageFrameCount, PMM_FIXED, startAddress) == EXIT_FAILURE)
            k_assertOnError ();

    }
}
/***************************************************************************************************
 * Deallocates specified pages starting from the specified physical location.
 *
 * @param startAddress  Physical memory location of the first page. Error is generated if not a
 *                      aligned to page boundary.
 * @param pageCount     Number of page frames to deallocate.
 * @return              If successful, the number of pages freed is returned. Should be equal to
 *                      pageCount.
 * @return              If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                      1. ERR_WRONG_ALIGNMENT  - startAddress not aligned to page boundary.
 **************************************************************************************************/
INT kpmm_free (PHYSICAL startAddress, UINT pageCount)
{
    if (IS_ALIGNED (startAddress.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, EXIT_FAILURE);

    /* Note: As startAddress is already aligned, both floor or ceiling are same here. */
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (startAddress.val);
    if (s_allocateDeallocatePages (startPageFrame, pageCount, false) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return pageCount;
}

/***************************************************************************************************
 * Searches the PAB and allocated the specified number of consecutive free physical pages.
 *
 * @param allocated     If successful, holds the allocated physical address. If type is PMM_FIXED,
 *                      this can be NULL.
 * @param pageCount     Number of byte frames to allocate.
 * @param type          PMM_AUTOMATIC to automatically find a suitable physical address.
 * @param type          PMM_FXED to allocate the specified number of bytes at the specified address.
 * @param start         Physical address where allocation will be attempted if type is PMM_FIXED.
 *                      Omitted otherwise.
 * @return              If successful, returns EXIT_SUCCESS.
 * @return              If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                      1. ERR_OUT_OF_MEM   - Could not find the required number of free
 *                                            consecutive pages.
 *                      2. ERR_WRONG_ALIGNMENT - 'start' is not aligned to page boundary.
 **************************************************************************************************/
INT kpmm_alloc (PHYSICAL *allocated, UINT pageCount, PMMAllocationTypes type, PHYSICAL start)
{
    UINT startPageFrame = 0;
    INT found = FALSE;

    switch (type)
    {
    case PMM_AUTOMATIC:
        for ( ; found == FALSE && startPageFrame < MAX_ADDRESSABLE_PAGE_COUNT
              ; startPageFrame++)
        {
            found = s_isPagesFree (startPageFrame, pageCount);
        }

        --startPageFrame; // undoing the last increment.
        break;
    case PMM_FIXED:
        if (IS_ALIGNED (start.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
            RETURN_ERROR (ERR_WRONG_ALIGNMENT, EXIT_FAILURE);

        /* Note: As 'start' is already aligned, both floor or ceiling are same here. */
        startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (start.val);
        found = s_isPagesFree (startPageFrame, pageCount);
        break;
    default:
        // Should not be here.
        k_assert (false, "Should not be here. Invalid Allocation Type");
    }

    // Not enough free pages found.
    if (found == FALSE) goto exitOutOfMemory;
    if (found == EXIT_FAILURE) goto exitError;

    // Free pages found. Now Allocate them.
    if (s_allocateDeallocatePages (startPageFrame, pageCount, true) == EXIT_FAILURE)
        goto exitError;

    USYSINT startAddress = PAGEFRAMES_TO_BYTES (startPageFrame);
    if ((type == PMM_AUTOMATIC || (type == PMM_FIXED && allocated != NULL)))
        (*allocated).val = startAddress;

    return EXIT_SUCCESS;
exitOutOfMemory:
    RETURN_ERROR (ERR_OUT_OF_MEM, EXIT_FAILURE);
exitError:
    return EXIT_FAILURE;
}

/***************************************************************************************************
 * Marks physical memory pages as either Allocated or Free in PAB.
 *
 * 'frameCount' pages of memory starting from 'startPageFrame' will be either marked as Allocated
 * or Free.
 *
 * @param startPageFrame    Physical page where the allocation must begin. First page frame is 0.
 * @param frameCount        Number of page frames to allocate. Out of Memory is generated if more
 *                          pages are requested than are free.
 * @param allocate          true to Allocate, false to deallocate.
 * @return                  On Success, EXIT_SUCCESS is returned.
 * @return                  If failure EXIT_FAILURE is returned. k_errorNumber is set with
 *                          error code.
 **************************************************************************************************/
static INT s_allocateDeallocatePages (UINT startPageFrame, UINT frameCount, bool allocate)
{
    kdebug_printf ("\r\n%s 0x%px bytes starting physical address 0x%px."
                    , (allocate) ? "Allocating" : "Freeing"
                    , PAGEFRAMES_TO_BYTES(frameCount), PAGEFRAMES_TO_BYTES(startPageFrame));

    INT success = EXIT_SUCCESS;
    UINT endPageFrame = startPageFrame + frameCount - 1;
    for (UINT pageFrame = startPageFrame
            ; pageFrame <= endPageFrame && success == EXIT_SUCCESS
            ; pageFrame++)
    {
        success = (allocate) ? s_allocatedPage (pageFrame)
                             : s_freePage (pageFrame);
    }

    return success;
}

/***************************************************************************************************
 * Checks if 'count' number of consecutive frames are free.
 *
 * @param startPageFrame    Physical page frame to start the check from. First page frame is 0.
 * @param count             Number of consecutive page frames that need to be free.
 * @return                  Returns TRUE if required number of free pages were found at the physical
 *                          location.
 * @return                  Returns FALSE if required number of free pages were not found at the
 *                          physical location.
 * @return                  If failure EXIT_FAILURE is returned. k_errorNumber is set with
 *                          error code.
 **************************************************************************************************/
static INT s_isPagesFree (UINT startPageFrame, UINT count)
{
    INT isAllocated = 0;
    for (UINT i = 0 ; i < count && isAllocated == 0; i++)
        isAllocated = s_isPageAllocated (i + startPageFrame);

    return (isAllocated < 0) ? -1 : !isAllocated;
}

/***************************************************************************************************
 * Checks if a physical page frame is allocated.
 *
 * @param pageFrame Physical page frame to check. First page frame is 0.
 * @return          Returns FALSE is page is free.
 * @return          Returns TRUE is page is not free.
 * @return          If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                  1. ERR_OUTSIDE_ADDRESSABLE_RANGE - Outside memory addressable by PAB.
 **************************************************************************************************/
static INT s_isPageAllocated (UINT pageFrame)
{
    if (pageFrame > MAX_ADDRESSABLE_PAGE)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, EXIT_FAILURE);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);

    return (s_pab[byteIndex] & (U8)(1U << bitIndex)) >> bitIndex;
}

/***************************************************************************************************
 * Marks a physical page frame as free.
 *
 * @param pageFrame Physical page frame to free. First page frame is 0.
 * @return          On Success, EXIT_SUCCESS is returned.
 * @return          If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                  1. ERR_DOUBLE_FREE - Page frame already free.
 *                  2. ERR_OUTSIDE_ADDRESSABLE_RANGE - Outside memory addressable by PAB.
 **************************************************************************************************/
static INT s_freePage (UINT pageFrame)
{
    if (pageFrame > MAX_ADDRESSABLE_PAGE)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, EXIT_FAILURE);

    if (!s_isPageAllocated (pageFrame))
        RETURN_ERROR (ERR_DOUBLE_FREE, EXIT_FAILURE);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);

    s_pab[byteIndex] &= (U8)(~(1U << bitIndex));
    return EXIT_SUCCESS;
}

/***************************************************************************************************
 * Marks a physical page frame as allocated.
 *
 * @param pageFrame Physical page frame to allocate. First page frame is 0.
 * @return          On Success, EXIT_SUCCESS is returned.
 * @return          If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                  1. ERR_DOUBLE_ALLOC - Page frame is already allocated.
 *                  2. ERR_OUTSIDE_ADDRESSABLE_RANGE - Outside memory addressable by PAB.
 **************************************************************************************************/
static INT s_allocatedPage (UINT pageFrame)
{
    if (pageFrame > MAX_ADDRESSABLE_PAGE)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, EXIT_FAILURE);

    if (s_isPageAllocated (pageFrame))
        RETURN_ERROR (ERR_DOUBLE_ALLOC, EXIT_FAILURE);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);

    s_pab[byteIndex] |= (U8)(1U << bitIndex);
    return EXIT_SUCCESS;
}
