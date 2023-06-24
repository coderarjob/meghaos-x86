/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*  TODO: Need to check if use of 'last byte/page' and 'byte/page' count concepts are used correctly.
*  TODO: Make allocation and deallocation atomic.
*/

#include <pmm.h>
#include <panic.h>
#include <mem.h>
#include <moslimits.h>
#include <types.h>
#include <x86/paging.h>
#include <kdebug.h>
#include <kerror.h>

static bool s_isPagesFree (UINT startPageFrame, UINT count, bool isDMA);
static void s_managePages (UINT startPageFrame, UINT frameCount, bool allocate, bool isDMA);
static INT s_get (UINT pageFrame, bool isDMA);
static void s_set (UINT pageFrame, bool alloc, bool isDMA);

static U8 *s_pab = NULL;

static bool s_isInitialized = false;

/***************************************************************************************************
 * Initializes PAB array.
 *
 * @return nothing
 * @error   Panics if called after initialization.
 **************************************************************************************************/
void kpmm_init ()
{
    if (kpmm_isInitialized())
        k_panic ("%s", "Called after PMM initialization.");

    s_pab = (U8 *)CAST_PA_TO_VA (g_pab);
    k_memset (s_pab, 0xFF, PAB_SIZE_BYTES);

    kpmm_arch_init(s_pab);

    // PMM is now initialized
    s_isInitialized = true;
}

/***************************************************************************************************
 * Deallocates specified pages starting from the specified physical location.
 *
 * Note: free can be used even before PAB is initialized. This is to solve the chicken and egg
 * problem where to initialize PAB one needs to at least call free or alloc functions. May be there
 * is a better solution.
 *
 * Important: Calling free before PAB initialization, is undefined behaviour. To remedy, before
 * calling free, initialize the PAB to some known state - kpmm_init for example initializes the PAB
 * with all ones.
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
bool kpmm_free (Physical startAddress, UINT pageCount)
{
    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    // Check if address is within the max addressable range.
    USYSINT allocation_end_byte = startAddress.val + (pageCount * CONFIG_PAGE_FRAME_SIZE_BYTES) - 1;

    if (allocation_end_byte >= kpmm_getAddressableByteCount (false))
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, false);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (startAddress.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    // Note: As startAddress is already aligned, both floor or ceiling are same here.
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (startAddress.val);

    // DMA addressing range does not determine freeing. false if passed here becuuse we want free to
    // deallocate any address.
    s_managePages (startPageFrame, pageCount, false, false);

    return true;
}

/***************************************************************************************************
 * Tries to allocates pages starting at the physical address that is provided.
 *
 * Note: This function can only be called once PMM is initialized. This is because it reads PAB to
 * make verify if the there are enough free pages at the provided address.
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
 * @error               Panics if called before initialization.
 **************************************************************************************************/
bool kpmm_allocAt (Physical start, UINT pageCount, bool isDMA)
{
    if (!kpmm_isInitialized())
        k_panic ("%s", "Called before PMM initialization.");

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    // Check if address is within the max addressable range.
    USYSINT allocation_end_byte = start.val + (pageCount * CONFIG_PAGE_FRAME_SIZE_BYTES) - 1;
    if (allocation_end_byte >= kpmm_getAddressableByteCount (isDMA))
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, false);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (start.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    // Note: Because the address is aligned to page boundary, Floor or ceiling are the same.
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (start.val);

    // Check if all the pages can be allocated at the provided location.
    bool found = s_isPagesFree (startPageFrame, pageCount, isDMA);

    // Free pages were not found. But there was no error.
    if (found == false)
        RETURN_ERROR (ERR_DOUBLE_ALLOC, false);

    // Free pages found. Now Allocate them.
    s_managePages (startPageFrame, pageCount, true, isDMA);

    return true;
}

/***************************************************************************************************
 * Tries to allocates pages starting at the physical address that is provided.
 *
 * Note: This function can only be called once PMM is initialized. This is because it reads PAB to
 * make find pages which can be allocated.
 *
 * @Input pageCount     Number of byte frames to allocate.
 * @Input isDMA         Is allocating from the DMA memory range.
 * @return              If successful, returns the allocated physical address.
 * @return              If failure PHYSICAL_NULL is returned. k_errorNumber is set with error code.
 *                      1. ERR_OUT_OF_MEM   - Could not find the required number of free
 *                                            consecutive pages.
 *                      2. ERR_INVALID_ARGUMENT - pageCount is zero.
 * @error               Panics if called before initialization.
 **************************************************************************************************/
Physical kpmm_alloc (UINT pageCount, bool isDMA)
{
    if (!kpmm_isInitialized())
        k_panic ("%s", "Called before PMM initialization.");

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, PHYSICAL_NULL);

    UINT startPageFrame = 1;
    INT found = false;

    // Search PAB for a suitable location.
    UINT maxPageFrame = kpmm_getAddressablePageCount (isDMA) - pageCount;
    for (; found == false && startPageFrame <= maxPageFrame; startPageFrame++)
        found = s_isPagesFree (startPageFrame, pageCount, isDMA);

    --startPageFrame; // undoing the last increment.

    // Free pages were not found. But there was no error.
    if (found == false)
        RETURN_ERROR (ERR_OUT_OF_MEM, PHYSICAL_NULL);

    // Free pages found. Now Allocate them.
    if (found == true)
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
 * @return                  Nothing
 * @error                   On failure panic is triggered.
 **************************************************************************************************/
static void s_managePages (UINT startPageFrame, UINT frameCount, bool allocate, bool isDMA)
{
    kdebug_printf ("\r\nI: %s starting physical address 0x%px, length 0x%px bytes ."
                    , (allocate) ? "Allocating" : "Freeing"
                    , PAGEFRAMES_TO_BYTES(startPageFrame)
                    , PAGEFRAMES_TO_BYTES(frameCount));

    k_assert (frameCount > 0, "Page frame count cannot be zero.");

    for (UINT pageFrame = startPageFrame ; frameCount-- > 0; pageFrame++)
        s_set (pageFrame, allocate, isDMA);
}

/***************************************************************************************************
 * Checks if 'count' number of consecutive frames are free.
 *
 * @Input startPageFrame    Physical page frame to start the check from. First page frame is 0.
 * @Input count             Number of consecutive page frames that need to be free.
 * @return                  Returns true if required number of free pages were found at the physical
 *                          location.
 * @return                  Returns false if required number of free pages were not found at the
 *                          physical location.
 * @return                  Nothing
 * @error                   On failure panic is triggered.
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
 * @return                  Nothing
 * @error                   On failure panic is triggered.
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
 * @error           On failure panic is triggered.
 **************************************************************************************************/
static INT s_get (UINT pageFrame, bool isDMA)
{
    if (pageFrame == 0)
        k_panic ("Invalid access: Page %u", pageFrame);

    if (pageFrame > kpmm_getAddressablePageCount (isDMA))
        k_panic ("Access outside range. Page %u", pageFrame);

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
    UINT mask = (U8)(1U << bitIndex);

    return (s_pab[byteIndex] & (U8)mask) >> bitIndex;
}

/***************************************************************************************************
 * Gets the total number of physical page frames in the system.
 *
 * Actual addressable RAM is the minimum of the total system RAM and the maximum RAM supported by
 * the OS.
 *
 * @Input isDMA     Queried for DMA.
 * @return          Gets the maximum page frame count for DMA hardware. Otherwise returns the
 *                  maximum page frame count supported by the PAB.
 **************************************************************************************************/
UINT kpmm_getAddressablePageCount (bool isDMA)
{
    UINT maxPageCount = BYTES_TO_PAGEFRAMES_FLOOR(kpmm_getAddressableByteCount (isDMA));
    return maxPageCount;
}

/***************************************************************************************************
 * Returns status of PAB array initializes .
 *
 * PAB initialization is complete after kpmm_init has finished execution and there is no error.
 *
 * @return      true if PAB array was initialized and PMM is ready. false otherwise.
 **************************************************************************************************/
bool kpmm_isInitialized ()
{
    return s_isInitialized;
}

/***************************************************************************************************
 * Calculates the size of free system memory based on PAB
 *
 * @return      Free memory size in bytes.
 * @error       Panics if called before initialization.
 **************************************************************************************************/
size_t kpmm_getFreeMemorySize ()
{
    if (!kpmm_isInitialized())
        k_panic ("%s", "Called before PMM initialization.");

    size_t freeBytes = 0;
    UINT pageFrameCount = kpmm_getAddressablePageCount (false);

    for (UINT frame = 1; frame < pageFrameCount; frame++)
        freeBytes += s_get (frame, false) ? 0 : CONFIG_PAGE_FRAME_SIZE_BYTES;

    return freeBytes;
}
