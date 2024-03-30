/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process Management
 * ---------------------------------------------------------------------------
 */

#include <types.h>
#include <pmm.h>
#include <kerror.h>
#include <paging.h>
#include <kstdlib.h>
#include <kprocess.h>
#include <x86/kprocess.h>
#include <x86/gdt.h>

#define PROCESS_VA_START 0x00010000

bool kprocess_create (void* processStartAddress, SIZE binLengthBytes)
{
    // Allocate physical memory for new PD
    Physical newPD;
    if (!kpg_createNewPageDirectory (&newPD, PG_NEWPD_FLAG_COPY_KERNEL_PAGES |
                                                 PG_NEWPD_FLAG_RECURSIVE_MAP)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false); // Cannot create new process.
    }

    // Copy the program to a page aligned physical address
    Physical binPhy;
    if (kpmm_alloc (&binPhy, 1, PMM_REGION_ANY) == false) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false); // Physical memory allocation failed.
    }

    kpg_switchPageDirectory (newPD);

    if (!kpg_map (kpg_getcurrentpd(), PROCESS_VA_START, binPhy,
                  PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false); // Map failed
    }

    k_memcpy ((void*)PROCESS_VA_START, processStartAddress, binLengthBytes);
    return true; // Success
}

void kprocess_switch()
{
    jump_to_usermode (GDT_SELECTOR_UDATA, GDT_SELECTOR_UCODE, (void (*)())PROCESS_VA_START);
}
