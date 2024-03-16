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

#define SPACE_USED()                 ((PTR)s_next - s_start)
#define IS_SPACE_AVAILABLE(sz_bytes) ((SPACE_USED() + (sz_bytes)) < SALLOC_SIZE_BYTES)

static void* s_next = NULL; // Points to the start of next allocation.
static PTR s_start  = 0;    // Points to the start salloc buffer.

void salloc_init()
{
    FUNC_ENTRY();
    s_start = (PTR)salloc_arch_preAllocateMemory();
    s_next  = (void*)s_start;
}

// TODO: salloc should take flags for alignment requirements to meet various placement requirements.
// For example: Page boundary must not ocour within TSS.
void* salloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: 0x%px", bytes);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO ("Size after aligning: 0x%px", allocSize);

    if (!IS_SPACE_AVAILABLE (allocSize))
    {
        RETURN_ERROR (ERR_OUT_OF_MEM, NULL);
    }

    s_next = (void*)((PTR)s_next + allocSize);
    return (void*)((PTR)s_next - allocSize);
}

void* scalloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: 0x%px", bytes);

    void* mem = salloc (bytes);
    if (mem != NULL)
        k_memset (mem, 0, bytes);
    return mem;
}
