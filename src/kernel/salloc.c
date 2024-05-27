/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Static memory allocation
 *
 * Static memories are for permanent allocations.
 * --------------------------------------------------------------------------------------------------
 */
#include <kdebug.h>
#include <kerror.h>
#include <utils.h>
#include <types.h>
#include <kstdlib.h>
#include <memmanage.h>
#include <x86/kernel.h>

#define SPACE_USED()                 ((PTR)s_next - s_start)
#define IS_SPACE_AVAILABLE(sz_bytes) ((SPACE_USED() + (sz_bytes)-1) < SALLOC_SIZE_BYTES)

static void* s_next = NULL; // Points to the start of next allocation.
static PTR s_start  = 0;    // Points to the start salloc buffer.

/***************************************************************************************************
 * Initializes virtual & physical memory for salloc.
 *
 * @return    None
 **************************************************************************************************/
void salloc_init()
{
    FUNC_ENTRY();
    s_start = (PTR)salloc_arch_preAllocateMemory();
    s_next  = (void*)s_start;
    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_SALLOC_READY);
}

/***************************************************************************************************
 * Allocates at least 'bytes' number of bytes from the salloc memory.
 *
 * TODO: salloc should take flags for alignment requirements to meet various placement requirements.
 * For example: Page boundary must not ocour within TSS.
 *
 * @Input   bytes   Number of bytes to allocate.
 * @return          Poiter to the start of the allocated memory. Or NULL on failure.
 * @error           ERR_INVALID_RANGE - Input is outside valid range.
 * @error           ERR_OUT_OF_MEM    - There is less memory than requested.
 **************************************************************************************************/
void* salloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: %px", bytes);

    if (bytes == 0 || bytes > SALLOC_SIZE_BYTES)
        RETURN_ERROR (ERR_INVALID_RANGE, NULL);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO ("Size after aligning: %px", allocSize);

    if (!IS_SPACE_AVAILABLE (allocSize))
        RETURN_ERROR (ERR_OUT_OF_MEM, NULL);

    s_next = (void*)((PTR)s_next + allocSize);
    return (void*)((PTR)s_next - allocSize);
}

/***************************************************************************************************
 * Allocates at least 'bytes' number of bytes from the salloc memory. Then initializes the memory
 * with zeros.
 *
 * TODO: salloc should take flags for alignment requirements to meet various placement requirements.
 * For example: Page boundary must not ocour within TSS.
 *
 * @Input   bytes   Number of bytes to allocate.
 * @return          Poiter to the start of the allocated memory. Or NULL on failure.
 * @error           ERR_INVALID_RANGE - Input is outside valid range.
 * @error           ERR_OUT_OF_MEM    - There is less memory than requested.
 **************************************************************************************************/
void* scalloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: %px", bytes);

    if (bytes == 0 || bytes > SALLOC_SIZE_BYTES)
        RETURN_ERROR (ERR_INVALID_RANGE, NULL);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO ("Size after aligning: %px", allocSize);

    void* mem = salloc (allocSize);
    if (mem != NULL)
        k_memset (mem, 0, allocSize);

    return mem;
}

/***************************************************************************************************
 * Gets the amount of memory allocated by salloc
 *
 * @return          Amount of allocated memory in bytes.
 **************************************************************************************************/
SIZE salloc_getUsedMemory()
{
    FUNC_ENTRY();

    SIZE usedSz = (PTR)s_next - (PTR)s_start;

    return usedSz;
}
