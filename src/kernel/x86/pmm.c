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
static void s_allocateDeallocateRange (USYSINT startAddress, USYSINT byteCount, bool allocate);

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
    k_memset (s_pab, 0x00, PAB_SIZE_BYTES);

    // If less RAM is available than MAX_ADDRESSABLE_BYTE, then mark this excess as NOT FREE in
    // PAB.
    BootLoaderInfo *bootloaderinfo = kboot_getCurrentBootLoaderInfo ();
    USYSINT systemRAM = kboot_calculateAvailableMemory (bootloaderinfo);
    USYSINT length_bytes = MAX_ADDRESSABLE_BYTE - systemRAM;
    kpmm_alloc (NULL, &length_bytes, PMM_FIXED, systemRAM);
}


/***************************************************************************************************
 * Deallocates specified pages starting from the specified physical location.
 *
 * @param startAddress  Physical memory location of the first page. Assert is thrown if not aligned 
 *                      to CONFIG_PAGE_FRAME_SIZE_BYTES bytes.
 * @param byteCount     Number of bytes to deallocate. Assert is thrown if not aligned to 
 *                      CONFIG_PAGE_FRAME_SIZE_BYTES bytes.
 * @return true         No error. Deallocation was successful.
 * @return false        Deallocation was not successful. k_errorNumber is set.
 **************************************************************************************************/
bool kpmm_free (PHYSICAL startAddress, USYSINT byteCount)
{
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

    *byteCount = ALIGN (*byteCount, CONFIG_PAGE_FRAME_SIZE_BYTES);
    UINT pageFrameCount = BYTES_TO_PAGEFRAMES (*byteCount);

    switch (type)
    {
    case PMM_AUTOMATIC:
        for ( ; found == false && startPageFrame < MAX_ADDRESSABLE_PAGE ; startPageFrame++)
            found = s_isPagesFree (startPageFrame, pageFrameCount);
        break;
    case PMM_FIXED:
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
    USYSINT frameCount = PAGEFRAMES_TO_BYTES (pageFrameCount);
    s_allocateDeallocateRange (startAddress, frameCount, true);

    if ((type == PMM_AUTOMATIC || (type == PMM_FIXED && allocated != NULL)))
        (*allocated).val = startAddress;

    RETURN_ERROR (ERR_NONE, true);
notfound:
    RETURN_ERROR (ERR_PMM_OUT_OF_MEM, false);
}

/***************************************************************************************************
 * Marks physical memory pages as either Allocated or Free in PAB.
 *
 * CEILING (byteCount / CONFIG_PAGE_FRAME_SIZE_BYTES) pages of memory starting from 'startAddress'
 * will be marked as wither Allocated or Free.
 *
 * @param startAddress      Physical address where the allocation must begin. If not aligned to
 *                          CONFIG_PAGE_FRAME_SIZE_BYTES, assertion is generated.
 * @param byteCount         Number of bytes to allocate. Will be rounded to the next nearest
 *                          multiple of CONFIG_PAGE_FRAME_SIZE_BYTES.
 * @param allocate          true to Allocate, false to deallocate.
 * @return nothing
 **************************************************************************************************/
static void s_allocateDeallocateRange (USYSINT startAddress, USYSINT byteCount, bool allocate)
{
    byteCount = ALIGN (byteCount, CONFIG_PAGE_FRAME_SIZE_BYTES);

    USYSINT endAddress = startAddress + byteCount - 1;
    UINT startPage = BYTES_TO_PAGEFRAMES (startAddress);
    UINT endPage = BYTES_TO_PAGEFRAMES (endAddress);

    kdebug_printf ("\r\n%s 0x%px bytes starting physical address 0x%px."
                    , (allocate) ? "Allocating" : "Freeing"
                    , byteCount, startAddress);

    k_assert (IS_ALIGNED (startAddress, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address not aligned.");
    k_assert (endAddress <= MAX_ADDRESSABLE_BYTE, "Out of range");

    for (UINT pageFrame = startPage
            ; pageFrame <= endPage
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
    k_assert (false == s_isPageAllocated (pageFrame), "Double free");

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
