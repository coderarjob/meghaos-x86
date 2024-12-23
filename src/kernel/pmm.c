/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*  TODO: Make allocation and deallocation atomic.
*/

#include <pmm.h>
#include <panic.h>
#include <moslimits.h>
#include <types.h>
#include <kdebug.h>
#include <kerror.h>
#include <bitmap.h>
#include <utils.h>
#include <kernel.h>
#include <memloc.h>

typedef struct PhysicalMemoryRegion
{
    Bitmap bitmap;
    Physical start;
    USYSINT length;
} PhysicalMemoryRegion;

static PhysicalMemoryRegion s_pmm_completeRegion = {0};
static U8 *s_pab = NULL;
static UINT kpmm_getUsableMemoryPagesCount(KernelPhysicalMemoryRegions reg);

static PhysicalMemoryRegion *s_getBitmapFromRegion (KernelPhysicalMemoryRegions reg)
{
    switch (reg) {
        case PMM_REGION_ANY: return &s_pmm_completeRegion;
        default: UNREACHABLE();
    }
    return NULL;
}

/***************************************************************************************************
 * Called when setting a bitmap state. It returns true if the change is allowed.
 * Any invalid condition here is unlikely so panic is justified.
 *
 * @Input pageFrame     Bitmap state index.
 * @Input old           Old state at the index.
 * @Input new           State to be set at the index.
 * @return              True is change can be allowed, false otherwise.
***************************************************************************************************/
static bool s_verifyChange(UINT pageFrame, BitmapState old, BitmapState new)
{
    if (pageFrame > kpmm_getUsableMemoryPagesCount (PMM_REGION_ANY))
        k_panic ("Access outside range. Page %u", pageFrame);

    if (old == PMM_STATE_USED && new == PMM_STATE_USED)
        k_panic ("Double allocation: Page %u", pageFrame);

    if (old == PMM_STATE_FREE && new == PMM_STATE_FREE)
        k_panic ("Double free: Page %u", pageFrame);

    if (KERNEL_PHASE_CHECK (KERNEL_PHASE_STATE_PMM_READY) && old == PMM_STATE_RESERVED)
        k_panic ("Use of Reserved page: Page %u", pageFrame);

    if (old == PMM_STATE_INVALID)
        k_panic ("Use of Invalid page: Page %u", pageFrame);

    return true;
}

/***************************************************************************************************
 * Total number of usable page frames in a memory region.
 * Returns 0 if region is outside the installed physical memory range.
 *
 * @Input reg       Physical memory region.
 * @return          The number of usable page frames in the memory region. 0 is there are no usable
 *                  page frames for this region.
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
    FUNC_ENTRY();

    s_pab = (U8 *)ARCH_MEM_START_KERNEL_PAB;

    s_pmm_completeRegion.bitmap.allow = s_verifyChange;
    s_pmm_completeRegion.bitmap.bitmap = s_pab;
    s_pmm_completeRegion.bitmap.bitsPerState = PAB_BITS_PER_STATE;
    s_pmm_completeRegion.bitmap.size = PAB_SIZE_BYTES;
    s_pmm_completeRegion.length = MAX_PAB_ADDRESSABLE_BYTE_COUNT;
    s_pmm_completeRegion.start = createPhysical(0);

    kpmm_arch_init(&s_pmm_completeRegion.bitmap);

    // PMM is now initialized
    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_PMM_READY);
}

/***************************************************************************************************
 * Deallocates specified pages starting from the specified physical location.
 *
 * Note: free can be used even before PAB is fully initialized. This is to solve the chicken and egg
 * problem where to initialize PAB one needs to at least call free or alloc functions. May be there
 * is a better solution.
 *
 * @Input startAddress  Physical memory location of the first page. Must be page aligned.
 * @Input pageCount     Number of pages to deallocate.
 * @return              If successful returns true, otherwise false and error code is set.
 **************************************************************************************************/
bool kpmm_free (Physical startAddress, UINT pageCount)
{
    FUNC_ENTRY("startAddress = %x, pageCount = %u", startAddress, pageCount);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_PMM_READY);

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

    PhysicalMemoryRegion *region = s_getBitmapFromRegion(PMM_REGION_ANY);
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
 * @Input start         Pages will be allocated from this physical address. Must be page aligned.
 * @Input pageCount     Number of byte frames to allocate.
 * @Input reg           Physical memory region.
 * @return              If successful returns true, otherwise false and error code is set.
 **************************************************************************************************/
bool kpmm_allocAt (Physical start, UINT pageCount, KernelPhysicalMemoryRegions reg)
{
    FUNC_ENTRY("start = %x, pageCount = %u, region = %u", start.val, pageCount, reg);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_PMM_READY);

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

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
    PhysicalMemoryRegion *region = s_getBitmapFromRegion(reg);

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
 * Searches for free physical pages and allocates contiguous pages as requested. The returned
 * address is the start of the first allocated page.
 *
 * Note: This function can only be called once PMM is initialized. This is because it reads PAB to
 * make find pages which can be allocated.
 *
 * @Output address      Allocated physical address. Is always page aligned.
 * @Input  pageCount    Number of byte frames to allocate.
 * @Input  reg          Physical memory region.
 * @return              If successful returns true, otherwise returns false and error code is set.
 **************************************************************************************************/
bool kpmm_alloc (Physical *address, UINT pageCount, KernelPhysicalMemoryRegions reg)
{
    FUNC_ENTRY("pageCount = %u, region = %u", pageCount, reg);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_PMM_READY);

    if (pageCount == 0)
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    PhysicalMemoryRegion *region = s_getBitmapFromRegion(reg);

    // Search PAB for a suitable location.
    INT pageFrame = bitmap_findContinous(&region->bitmap, PMM_STATE_FREE, pageCount);
    if (pageFrame == KERNEL_EXIT_FAILURE)
        RETURN_ERROR (ERR_OUT_OF_MEM, false);

    k_assert((UINT)pageFrame < kpmm_getUsableMemoryPagesCount(reg), "Out of range");

    // Free pages found. Now Allocate them.
    if (bitmap_setContinous(&region->bitmap,
                                (UINT)pageFrame,
                                pageCount,
                                PMM_STATE_USED))
    {
        *address = createPhysical(PAGEFRAME_TO_PHYSICAL((UINT) pageFrame));
        INFO("Allocated address = %x", address->val);
        k_assert (IS_ALIGNED (address->val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");
        return true;
    }

    UNREACHABLE();
    NORETURN();
}

/***************************************************************************************************
 * Calculates the size of free system memory based on PAB
 *
 * @return      Free memory size in bytes.
 * @error       Panics if called before initialization.
 **************************************************************************************************/
size_t kpmm_getFreeMemorySize ()
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_PMM_READY);

    UINT usablePageCount = kpmm_getUsableMemoryPagesCount(PMM_REGION_ANY);
    PhysicalMemoryRegion *region = s_getBitmapFromRegion(PMM_REGION_ANY);

    size_t freePages = 0;
    for (UINT frame = 1; frame < usablePageCount; frame++) {
        if (bitmap_get (&region->bitmap, frame) == PMM_STATE_FREE)
            freePages++;
    }

    k_assert(freePages <= kpmm_getUsableMemoryPagesCount(PMM_REGION_ANY), "Invalid frames");
    return PAGEFRAME_TO_PHYSICAL(freePages);
}

/***************************************************************************************************
 * Total amount of usable bytes in the memory region.
 * If region completely falls outsize the installed RAM, then the usable size of the region is zero.
 *
 * @Input   reg     Physical memory region
 * @return          Amount of a usable/accessible memory for the region.
 **************************************************************************************************/
USYSINT kpmm_getUsableMemorySize (KernelPhysicalMemoryRegions reg)
{
//    FUNC_ENTRY("region = %u", reg);

    static S64 regionLength = -1; // static to cache the result. Amount of memory will not change.

#if !defined(UNITTEST)
    // TODO: This optimization works in real life, because the amount of RAM will not change while
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

 //   INFO ("Region length = %px bytes", regionLength);
    return (USYSINT) regionLength;
}

/***************************************************************************************************
 * Queries to find the status of a physical page.
 *
 * @Input   Physical                    Physical memory address. Must be page aligned.
 * @return  KernelPhysicalMemoryStates  State of the physical memory.
 **************************************************************************************************/
KernelPhysicalMemoryStates kpmm_getPageStatus (Physical phy)
{
    FUNC_ENTRY ("Physical address = %x", phy);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_PMM_READY);

    // Check alignment of the address. Must be aligned to page boundary.
    if (IS_ALIGNED (phy.val, CONFIG_PAGE_FRAME_SIZE_BYTES) == false) {
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, PMM_STATE_INVALID);
    }

    // Note: As startAddress is already aligned, both floor or ceiling are same here.
    UINT pageFrame = BYTES_TO_PAGEFRAMES_FLOOR (phy.val);

    PhysicalMemoryRegion* reg = s_getBitmapFromRegion (PMM_REGION_ANY);
    return bitmap_get (&reg->bitmap, pageFrame);
}
