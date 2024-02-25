/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Static memory allocation
 *
 * Static memories are for permanent allocations.
 * --------------------------------------------------------------------------------------------------
 */
#include <kerror.h>
#include <stdbool.h>
#include <utils.h>
#include <types.h>
#include <config.h>
#include <pmm.h>
#include <panic.h>
#include <paging.h>
#include <mem.h>

/* TODO: Move to x86/memloc.h
 * TODO: Create some way to keep memory start and size. */
#define SALLOC_MEM_START  (3 * GB + 2 * MB)
#define SALLOC_MEM_SIZE   (1 * MB)
#define SALLOC_GRANUALITY (8 * Byte)
void *next = NULL; // Points to the start of next allocation.

void salloc_init()
{
    /* Pre Map SALLOC_PA_MEM_START with SALLOC_MEM_START in the kernel. */
    // paging_map (PD, UNBACKED, SALLOC_MEM_START, SALLOC_MEM_SIZE, UNUSED);

    /* Physically back 1 page for the heap */
    Physical pa;
    if (kpmm_alloc (&pa, 1, PMM_REGION_ANY) == false)
        k_panic ("%s", "Memory allocaiton failed");

    PageDirectory pd = kpg_getcurrentpd();
    if (kpg_map (pd, SALLOC_MEM_START, pa, PG_MAP_FLAG_KERNEL) == false)
        k_panic ("%s", "Page map failed");

    next = (void *)SALLOC_MEM_START;
}

// TODO: salloc should take flags for alignment requirements to meet various placement requirements.
// For example: Page boundary must not ocour within TSS.
void *salloc (UINT bytes)
{
    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);

    /*if (paging_ismapped(next, bytes) == false)
    {
        // NOTE: New PT may need to be created here and associate it with the current PD. Later on
        // with demand creation, this will not be required; salloc_init will map unbacked PTs for
        // the whole Heap address space.
        Physical pa;
        if (kpmm_alloc(&pa, 1, PMM_REGION_ANY) == false)
            k_panic("%s", "Memory allocaiton failed");

        //paging_map (paging_getCurrentPD(), BACKED, next, 4096, &pa);
    }*/

    if ((PTR)next < (SALLOC_MEM_START + SALLOC_MEM_SIZE))
    {
        next = (void *)((PTR)next + allocSize);
        return (void *)((PTR)next - allocSize);
    }

    RETURN_ERROR (ERR_OUT_OF_MEM, NULL);
}

void *scalloc (UINT bytes)
{
    void *mem = salloc (bytes);
    if (mem != NULL)
        k_memset(mem, 0, bytes);
    return mem;
}

