/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* --------------------------------------------------------------------------------------------------
*/

#include <kernel.h>

static void s_allocatedPage (UINT pageFrame);
static void s_freePage (UINT pageFrame);
static bool s_isPageAllocated (UINT pageFrame);
static bool s_isPagesFree (UINT startPageFrame, UINT count);
static void s_allocateDeallocateRange (USYSINT phStartAddress, USYSINT byteCount, bool allocate);

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

    // If less RAM is available than MAX_ADDRESSABLE_BYTE, then mark this excess as NOT FREE in
    // PAB.
    BootLoaderInfo *bootloaderinfo = kboot_getCurrentBootLoaderInfo ();
    USYSINT availableRAMBytes = kboot_calculateAvailableMemory (bootloaderinfo);
    USYSINT length_bytes = MAX_ADDRESSABLE_BYTE - availableRAMBytes;
    //kpmm_alloc (NULL, length_bytes, PMM_FIXED, availableRAMBytes);
    s_allocateDeallocateRange (availableRAMBytes, length_bytes, true);
}


/***************************************************************************************************
 * Searches the PAB and allocated the specified number of consecutive free physical pages.
 *
 * @param allocated     If successful, holds the allocated physical address. If type is PMM_FIXED,
 *                      this can be NULL.
 * @param byteCount     Number of bytes to allocate. Will be rounded to the next nearest multiple
 *                      of CONFIG_PAGE_FRAME_SIZE_BYTES.
 * @param type          PMM_AUTOMATIC to automatically find a suitable physical address.
 * @param type          PMM_FXED to allocate the specified number of bytes at the specified address.
 * @param start         Physical address where allocation will be attempted if type is PMM_FIXED.
 *                      Omitted otherwise.
 * @return true         No error. Allocation was successful.
 * @return false        Out of Memory. Allocation was not successful. k_errorNumber is set.
 **************************************************************************************************/
bool kpmm_alloc (PHYSICAL_ADDRESS *allocated, USYSINT byteCount , PMMAllocationTypes type,
                PHYSICAL_ADDRESS start)
{
    UINT startPageFrame = 0;
    bool found = false;

    byteCount = ALIGN (byteCount, CONFIG_PAGE_FRAME_SIZE_BYTES);
    UINT pageFrameCount = BYTES_TO_PAGEFRAMES (byteCount);

    switch (type)
    {
    case PMM_AUTOMATIC:
        for ( ; found == false && startPageFrame < MAX_ADDRESSABLE_PAGE ; startPageFrame++)
            found = s_isPagesFree (startPageFrame, pageFrameCount);
        break;
    case PMM_FIXED:
        startPageFrame = BYTES_TO_PAGEFRAMES (start.phy_addr);
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
    USYSINT length_frames = PAGEFRAMES_TO_BYTES (pageFrameCount);
    s_allocateDeallocateRange (startAddress, length_frames, true);

    if ((type == PMM_AUTOMATIC || (type == PMM_FIXED && allocated != NULL)))
        (*allocated).phy_addr = startAddress;

    RETURN_ERROR (ERR_NONE, true);
notfound:
    RETURN_ERROR (ERR_PMM_OUT_OF_MEM, false);
}

/***************************************************************************************************
 * Marks physical memory pages as either Allocated or Free in PAB.
 *
 * CEILING (length / CONFIG_PAGE_FRAME_SIZE_BYTES) pages of memory starting from 'startAddress' will
 * be marked as wither Allocated or Free.
 *
 * @param phStartAddress    Physical address where the allocation must begin. If not aligned to
 *                          CONFIG_PAGE_FRAME_SIZE_BYTES, assertion is generated.
 * @param byteCount         Number of bytes to allocate. Will be rounded to the next nearest
 *                          multiple of CONFIG_PAGE_FRAME_SIZE_BYTES.
 * @param allocate          true to Allocate, false to deallocate.
 * @return nothing
 **************************************************************************************************/
static void s_allocateDeallocateRange (USYSINT phStartAddress, USYSINT byteCount, bool allocate)
{
    byteCount = ALIGN (byteCount, CONFIG_PAGE_FRAME_SIZE_BYTES);

    USYSINT phEndAddress = phStartAddress + byteCount - 1;
    UINT phStartPage = BYTES_TO_PAGEFRAMES (phStartAddress);
    UINT phEndPage = BYTES_TO_PAGEFRAMES (phEndAddress);

    k_assert (IS_ALIGNED (phStartAddress, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address not aligned.");
    k_assert (phEndAddress <= MAX_ADDRESSABLE_BYTE, "Out of range");

    for (UINT pageFrame = phStartPage
            ; pageFrame <= phEndPage
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

    return isFree == false;
}

/***************************************************************************************************
 * Checks if a physical page frame is allocated.
 *
 * @param pageFrame Physical page frame to check.
 * @return true     if the page frame is allocated.
 * @return false    if the page frame is free.
 **************************************************************************************************/
static bool s_isPageAllocated (UINT pageFrame)
{
    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
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
    k_assert (false == s_isPageAllocated (pageFrame), "Double free");

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
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
    k_assert (s_isPageAllocated (pageFrame), "Double allocation");

    UINT byteIndex = (pageFrame / 8);
    UINT bitIndex = (pageFrame % 8);
    s_pab[byteIndex] |= (U8)(1U << bitIndex);
}
