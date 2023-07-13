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
#include <common/bitmap.h>

typedef enum PhysicalMemoryBitmapState
{
    PMM_INIT_STATE_UNINITIALIZED,
    PMM_INIT_STATE_PARTIAL,
    PMM_INIT_STATE_COMPLETE
} PhysicalMemoryBitmapState;

typedef struct PhysicalMemoryRegion
{
    Bitmap bitmap;
    Physical start;
    USYSINT length;
} PhysicalMemoryRegion;

static PhysicalMemoryRegion s_pmm_completeRegion;
static UINT kpmm_getUsableMemoryPagesCount(KernelPhysicalMemoryRegions reg);
static U8 *s_pab = NULL;
static PhysicalMemoryBitmapState s_isInitialized = PMM_INIT_STATE_UNINITIALIZED;

static PhysicalMemoryRegion *s_getBitmapFromRegion (KernelPhysicalMemoryRegions reg)
{
    switch (reg) {
        case PMM_REGION_ANY: return &s_pmm_completeRegion;
        default: k_assert(false, "Invalid region. Must not be here");
    }
    return NULL;
}

static bool s_verifyChange(UINT pageFrame, BitmapState old, BitmapState new)
{
    // TODO: These should not panic. They should instead set error code and return false.
    if (pageFrame == 0)
        k_panic ("Invalid access: Page %u", pageFrame);

    if (pageFrame > kpmm_getUsableMemoryPagesCount (PMM_REGION_ANY))
        k_panic ("Access outside range. Page %u", pageFrame);

    if (old == PMM_STATE_USED && new == PMM_STATE_USED)
        k_panic ("Double allocation: Page %u", pageFrame);

    if (old == PMM_STATE_FREE && new == PMM_STATE_FREE)
        k_panic ("Double free: Page %u", pageFrame);

    if (old == PMM_STATE_RESERVED)
        k_panic ("Use of Reserved page: Page %u", pageFrame);

    if (old == PMM_STATE_INVALID)
        k_panic ("Use of Invalid page: Page %u", pageFrame);

    return true;
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
static UINT kpmm_getUsableMemoryPagesCount(KernelPhysicalMemoryRegions reg)
{
    UINT maxPageCount = BYTES_TO_PAGEFRAMES_FLOOR(kpmm_getUsableMemorySize(reg));
    return maxPageCount;
}

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

    s_pmm_completeRegion.bitmap.allow = s_verifyChange;
    s_pmm_completeRegion.bitmap.bitmap = s_pab;
    s_pmm_completeRegion.bitmap.bitsPerState = PAB_BITS_PER_STATE;
    s_pmm_completeRegion.bitmap.size = PAB_SIZE_BYTES;
    s_pmm_completeRegion.length = MAX_PAB_ADDRESSABLE_BYTE_COUNT;
    s_pmm_completeRegion.start = createPhysical(0);

    // Partial initialized because free memory regions still needs to be marked in the bitmap.
    s_isInitialized = PMM_INIT_STATE_PARTIAL;

    kpmm_arch_init(s_pab);

    // PMM is now initialized
    s_isInitialized = PMM_INIT_STATE_COMPLETE;
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
    k_assert(s_isInitialized > PMM_INIT_STATE_UNINITIALIZED, "Called before PMM initialization.");

    PhysicalMemoryRegion *region = s_getBitmapFromRegion(PMM_REGION_ANY);

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    // Check if address is within the max addressable range.
    USYSINT allocation_end_byte = startAddress.val + (pageCount * CONFIG_PAGE_FRAME_SIZE_BYTES) - 1;
    if (allocation_end_byte >= kpmm_getUsableMemorySize(PMM_REGION_ANY))
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, false);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (startAddress.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    // Note: As startAddress is already aligned, both floor or ceiling are same here.
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (startAddress.val);

    bool success = bitmap_setContinous(&region->bitmap,
                                       startPageFrame,
                                       pageCount,
                                       PMM_STATE_FREE);

    return success;
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
bool kpmm_allocAt (Physical start, UINT pageCount, KernelPhysicalMemoryRegions reg)
{
    k_assert(kpmm_isInitialized(), "Called before PMM initialization.");

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    PhysicalMemoryRegion *region = s_getBitmapFromRegion(reg);

    // Check if address is within the max addressable range.
    USYSINT allocation_end_byte = start.val + (pageCount * CONFIG_PAGE_FRAME_SIZE_BYTES) - 1;
    if (allocation_end_byte >= kpmm_getUsableMemorySize (reg))
        RETURN_ERROR (ERR_OUTSIDE_ADDRESSABLE_RANGE, false);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (start.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false)
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    // Note: Because the address is aligned to page boundary, Floor or ceiling are the same.
    UINT startPageFrame = BYTES_TO_PAGEFRAMES_FLOOR (start.val);

    // Check if all the pages can be allocated at the provided location.
    bool found = bitmap_findContinousAt(&region->bitmap,
                                        PMM_STATE_FREE,
                                        pageCount,
                                        startPageFrame);

    // Free pages were not found. But there was no error.
    if (found == false)
        RETURN_ERROR (ERR_DOUBLE_ALLOC, false);

    // Free pages found. Now Allocate them.
    bool success = bitmap_setContinous (&region->bitmap,
                                        startPageFrame,
                                        pageCount,
                                        PMM_STATE_USED);
    return success;
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
Physical kpmm_alloc (UINT pageCount, KernelPhysicalMemoryRegions reg)
{
    k_assert(kpmm_isInitialized(), "Called before PMM initialization.");

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, PHYSICAL_NULL);

    PhysicalMemoryRegion *region = s_getBitmapFromRegion(reg);

    // Search PAB for a suitable location.
    INT pageFrame = bitmap_findContinous(&region->bitmap, PMM_STATE_FREE, pageCount);
    if (pageFrame == KERNEL_EXIT_FAILURE)
        RETURN_ERROR (ERR_OUT_OF_MEM, PHYSICAL_NULL);

    k_assert((UINT)pageFrame < kpmm_getUsableMemoryPagesCount(reg), "Out of range");

    // Free pages found. Now Allocate them.
    bool success = bitmap_setContinous(&region->bitmap,
                                (UINT)pageFrame,
                                pageCount,
                                PMM_STATE_USED);

    return success ? createPhysical(PAGEFRAMES_TO_BYTES((UINT)pageFrame)): PHYSICAL_NULL;

    // There was an error in either s_isPagesFree or s_managePages
    return PHYSICAL_NULL;
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
    return s_isInitialized == PMM_INIT_STATE_COMPLETE;
}

/***************************************************************************************************
 * Calculates the size of free system memory based on PAB
 *
 * @return      Free memory size in bytes.
 * @error       Panics if called before initialization.
 **************************************************************************************************/
size_t kpmm_getFreeMemorySize ()
{
    k_assert(kpmm_isInitialized(), "Called before PMM initialization.");

    UINT usablePageCount = kpmm_getUsableMemoryPagesCount(PMM_REGION_ANY);
    PhysicalMemoryRegion *region = s_getBitmapFromRegion(PMM_REGION_ANY);

    size_t freePages = 0;
    for (UINT frame = 1; frame < usablePageCount; frame++)
        if (bitmap_get(&region->bitmap, frame) == PMM_STATE_FREE)
        freePages++;

    return PAGEFRAMES_TO_BYTES(freePages);
}

/***************************************************************************************************
 * Gets the amount of usable RAM for a memory region.
 *
 * Usable memory size for the region. If region completely falls outsize the installed RAM, then the
 * usable size of the region is zero.
 *
 * @Input   reg     Region
 * @return          Amount of actual accessible RAM in bytes for the region.
 **************************************************************************************************/
USYSINT kpmm_getUsableMemorySize (KernelPhysicalMemoryRegions reg)
{
    static S64 regionLength = -1; // static to cache the result. Amount of memory will not change.

#if !defined(UNITTEST)
    // TODO: This optimazation works in real life, because the amount of RAM will not change while
    // the OS is running. However this is not true when unittesting. Currently we run all unittests
    // in the same process, which means this function would have always returned the same value
    // following the very first call.
    if (regionLength == -1)
#endif
    {
        U64 installedRamBytes = kpmm_arch_getInstalledMemoryByteCount();
        PhysicalMemoryRegion *region = s_getBitmapFromRegion(reg);
        S64 earliestMemoryEnd = (S64)MIN(installedRamBytes, region->start.val + region->length);
        regionLength = earliestMemoryEnd - region->start.val;
        if (regionLength < 0) regionLength = 0;
    }

    return (USYSINT) regionLength;
}
