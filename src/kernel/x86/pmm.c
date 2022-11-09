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

static INT s_isPagesFree (UINT startPageFrame, UINT count);
static INT s_managePages (UINT startPageFrame, UINT frameCount, bool allocate);
static void s_markMemoryOccupiedByModuleFiles ();
static void s_markFreeMemory ();
static INT s_get (UINT pageFrame);
static INT s_set (UINT pageFrame, bool alloc);

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

    USYSINT addr = 0;
    kpmm_alloc (NULL, 1, PMM_FIXED, addr);
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
    INT mmapCount = kboot_getBootLoaderInfoBootMemoryMapItemCount (bootloaderinfo);
    for (INT i = 0; i < mmapCount; i++)
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
        USYSINT lengthBytes_possible = (USYSINT)(endAddress - startAddress  + 1);
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
    INT filesCount = kboot_getBootLoaderInfoFilesCount (bootloaderinfo);
    for (INT i = 0; i < filesCount; i++)
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
 * @Input startAddress  Physical memory location of the first page. Error is generated if not a
 *                      aligned to page boundary.
 * @Input pageCount     Number of page frames to deallocate.
 * @return              If successful, returns EXIT_SUCCESS.
 * @return              If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                      1. ERR_WRONG_ALIGNMENT  - startAddress not aligned to page boundary.
 *                      2. ERR_INVALID_ARGUMENT - pageCount is zero.
 **************************************************************************************************/
INT kpmm_free (PHYSICAL startAddress, UINT pageCount)
{
    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, EXIT_FAILURE);

    if (IS_ALIGNED (startAddress.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, EXIT_FAILURE);

    /* Note: As startAddress is already aligned, both floor or ceiling are same here. */
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (startAddress.val);
    if (s_managePages (startPageFrame, pageCount, false) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/***************************************************************************************************
 * Searches the PAB and allocated the specified number of consecutive free physical pages.
 *
 * @Output allocated    If successful, holds the allocated physical address. If type is PMM_FIXED,
 *                      this can be NULL.
 * @Input pageCount     Number of byte frames to allocate.
 * @Input type          PMM_DMA to allocate enough free physical pages suitable for DMA.
 * @Input type          PMM_NORMAL to allocate enough free physical pages anywhere in memory.
 * @Input type          PMM_FXED to allocate enough free physical pages at the specified address.
 * @Input start         Pages will be allocated from this physical address, for PMM_FIXED type.
 *                      omitted otherwise. Must be page aligned.
 * @return              If successful, returns EXIT_SUCCESS.
 * @return              If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                      1. ERR_OUT_OF_MEM   - Could not find the required number of free
 *                                            consecutive pages.
 *                      2. ERR_WRONG_ALIGNMENT - 'start' is not aligned to page boundary.
 *                      3. ERR_DOUBLE_ALLOC - All or part of specified memory is already allocated.
 *                                            This error is only thrown for FIXED allocations.
 *                      4. ERR_INVALID_ARGUMENT - pageCount is zero.
 **************************************************************************************************/
INT kpmm_alloc (PHYSICAL *allocated, UINT pageCount, PMMAllocationTypes type, PHYSICAL start)
{
    UINT startPageFrame = 0;
    INT found = FALSE;

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, EXIT_FAILURE);

    switch (type)
    {
    case PMM_DMA:
    case PMM_NORMAL:
        {
            UINT maxPageCount = (type == PMM_NORMAL) ? MAX_ADDRESSABLE_PAGE_COUNT
                                                     : MAX_DMA_ADDRESSABLE_PAGE_COUNT;

            k_assert (maxPageCount <= MAX_ADDRESSABLE_PAGE_COUNT, "Outside addressable range");

            for ( ; found == FALSE && startPageFrame < maxPageCount; startPageFrame++)
                found = s_isPagesFree (startPageFrame, pageCount);

            --startPageFrame; // undoing the last increment.
            break;
        }
    case PMM_FIXED:
        {
            if (IS_ALIGNED (start.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
                RETURN_ERROR (ERR_WRONG_ALIGNMENT, EXIT_FAILURE);

            /* Note: As 'start' is already aligned, both floor or ceiling are same here. */
            startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (start.val);
            found = s_isPagesFree (startPageFrame, pageCount);
            break;
        }
    default:
        // Should not be here.
        k_assert (false, "Should not be here. Invalid Allocation Type");
    }

    // Not enough free pages found.
    if (found == EXIT_FAILURE) goto exitFailure;
    if (found == FALSE) goto exitError;

    // Free pages found. Now Allocate them.
    if (s_managePages (startPageFrame, pageCount, true) == EXIT_FAILURE)
        goto exitError;

    USYSINT startAddress = PAGEFRAMES_TO_BYTES (startPageFrame);
    if ((type != PMM_FIXED || (type == PMM_FIXED && allocated != NULL)))
        (*allocated).val = startAddress;

    return EXIT_SUCCESS;
exitError:
    RETURN_ERROR ((type == PMM_FIXED) ? ERR_DOUBLE_ALLOC : ERR_OUT_OF_MEM, EXIT_FAILURE);
exitFailure:
    return EXIT_FAILURE;
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
 * @return                  On Success, EXIT_SUCCESS is returned.
 * @return                  If failure EXIT_FAILURE is returned. k_errorNumber is set with
 *                          error code.
 *                          1. ERR_DOUBLE_ALLOC - A page frame is already allocated.
 *                          2. ERR_DOUBLE_FREE - A page frame is already free.
 **************************************************************************************************/
static INT s_managePages (UINT startPageFrame, UINT frameCount, bool allocate)
{
    kdebug_printf ("\r\nI: %s 0x%px bytes starting physical address 0x%px."
                    , (allocate) ? "Allocating" : "Freeing"
                    , PAGEFRAMES_TO_BYTES(frameCount), PAGEFRAMES_TO_BYTES(startPageFrame));

    k_assert (frameCount > 0, "Page frame count cannot be zero.");

    UINT endPageFrame = startPageFrame + frameCount - 1;
    for (UINT pageFrame = startPageFrame
            ; pageFrame <= endPageFrame
            ; pageFrame++)
    {
        INT bit;
        if ((bit = s_get (pageFrame)) < 0)
                goto exitFailure;

        if (allocate && bit == 1)
            RETURN_ERROR (ERR_DOUBLE_ALLOC, EXIT_FAILURE);

        if (!allocate && bit == 0)
            RETURN_ERROR (ERR_DOUBLE_FREE, EXIT_FAILURE);

        if (s_set (pageFrame, allocate) < 0)
            goto exitFailure;
    }

    return EXIT_SUCCESS;
exitFailure:
    return EXIT_FAILURE;
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
 * @return                  If failure EXIT_FAILURE is returned. k_errorNumber is set with
 *                          error code.
 **************************************************************************************************/
static INT s_isPagesFree (UINT startPageFrame, UINT count)
{
    k_assert (count > 0, "Page frame count cannot be zero.");

    INT isAllocated = 0;
    for (UINT i = 0 ; i < count && isAllocated == 0; i++)
        isAllocated = s_get (i + startPageFrame);

    return (isAllocated < 0) ? EXIT_FAILURE : !isAllocated;
}

/***************************************************************************************************
 * Sets/Clears corresponding page frame bit in PAB.
 *
 * @Input pageFrame Physical page frame to change. First page frame is 0.
 * @return          On Success, EXIT_SUCCESS is returned.
 * @return          If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                  1. ERR_OUTSIDE_ADDRESSABLE_RANGE - Outside memory addressable by PAB.
 **************************************************************************************************/
static INT s_set (UINT pageFrame, bool alloc)
{
    if (pageFrame > MAX_ADDRESSABLE_PAGE)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, EXIT_FAILURE);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
    UINT mask = (U8)(1U << bitIndex);

    if (alloc)
        s_pab[byteIndex] |= (U8)mask;
    else
        s_pab[byteIndex] &= (U8)~mask;
    return EXIT_SUCCESS;
}

/***************************************************************************************************
 * Gets corresponding page frame bit from PAB.
 *
 * @Input pageFrame Physical page frame to query. First page frame is 0.
 * @return          On Success, returns 0 is page frame is free, 1 otherwise.
 * @return          If failure EXIT_FAILURE is returned. k_errorNumber is set with error code.
 *                  1. ERR_OUTSIDE_ADDRESSABLE_RANGE - Outside memory addressable by PAB.
 **************************************************************************************************/
static INT s_get (UINT pageFrame)
{
    if (pageFrame > MAX_ADDRESSABLE_PAGE)
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, EXIT_FAILURE);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);

    return (s_pab[byteIndex] & (U8)(1U << bitIndex)) >> bitIndex;
}
