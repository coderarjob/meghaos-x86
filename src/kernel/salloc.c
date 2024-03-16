/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Static memory allocation
 *
 * Static memories are for permanent allocations.
 * --------------------------------------------------------------------------------------------------
 */
#include <kdebug.h>
#include <assert.h>
#include <config.h>
#include <kerror.h>
#include <stdbool.h>
#include <utils.h>
#include <types.h>
#include <config.h>
#include <pmm.h>
#include <panic.h>
#include <paging.h>
#include <kstdlib.h>
#include <memmanage.h>
#if defined(__i386__)
    #include <x86/memloc.h>
#endif

#define SALLOC_MEM_END (SALLOC_MEM_START + SALLOC_SIZE_BYTES)

static void* s_next = NULL; // Points to the start of next allocation.

void salloc_init()
{
    FUNC_ENTRY();

    s_next = salloc_arch_preAllocateMemory();
}

// TODO: salloc should take flags for alignment requirements to meet various placement requirements.
// For example: Page boundary must not ocour within TSS.
void* salloc (UINT bytes)
{
    FUNC_ENTRY ("Bytes: 0x%px", bytes);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO ("Size after aligning: 0x%px", allocSize);

    if ((PTR)s_next >= SALLOC_MEM_END)
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
