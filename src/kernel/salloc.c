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
#include <kernel.h>
#include <memloc.h>

#define SPACE_USED()                 ((PTR)s_next - s_start)
#define IS_SPACE_AVAILABLE(sz_bytes) ((SPACE_USED() + (sz_bytes)-1) < ARCH_MEM_LEN_BYTES_SALLOC)

static void* s_next = NULL; // Points to the start of next allocation.
static PTR s_start  = 0;    // Points to the start salloc buffer.

/***************************************************************************************************
 * Initializes virtual & physical memory for salloc.
 *
 * @return    None
 **************************************************************************************************/
void ksalloc_init()
{
    FUNC_ENTRY();
    s_start = (PTR)ARCH_MEM_START_SALLOC;
    s_next  = (void*)s_start;
    KERNEL_PHASE_SET (KERNEL_PHASE_STATE_SALLOC_READY);

    INFO ("salloc starts at: %px", s_start);
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
void* ksalloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: %px", bytes);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_SALLOC_READY);

    if (bytes == 0 || bytes > ARCH_MEM_LEN_BYTES_SALLOC)
        RETURN_ERROR (ERR_INVALID_RANGE, NULL);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO ("Size after aligning: %px", allocSize);

    if (!IS_SPACE_AVAILABLE (allocSize))
        RETURN_ERROR (ERR_OUT_OF_MEM, NULL);

    INFO ("Allocated at: %x", s_next);

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
void* kscalloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: %px", bytes);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_SALLOC_READY);

    if (bytes == 0 || bytes > ARCH_MEM_LEN_BYTES_SALLOC)
        RETURN_ERROR (ERR_INVALID_RANGE, NULL);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO ("Size after aligning: %px", allocSize);

    void* mem = ksalloc (allocSize);
    if (mem != NULL)
        k_memset (mem, 0, allocSize);

    return mem;
}

/***************************************************************************************************
 * Gets the amount of memory allocated by salloc
 *
 * @return          Amount of allocated memory in bytes.
 **************************************************************************************************/
SIZE ksalloc_getUsedMemory()
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_SALLOC_READY);

    SIZE usedSz = (PTR)s_next - (PTR)s_start;

    return usedSz;
}
