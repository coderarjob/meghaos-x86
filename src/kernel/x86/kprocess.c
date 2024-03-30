/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process Management
 * ---------------------------------------------------------------------------
 */

#include <kdebug.h>
#include <types.h>
#include <pmm.h>
#include <kerror.h>
#include <paging.h>
#include <kstdlib.h>
#include <kprocess.h>
#include <x86/kprocess.h>
#include <x86/gdt.h>
#include <memmanage.h>

#define PROCESS_TEXT_VA_START  0x00010000
#define PROCESS_STACK_VA_START 0x00030000
#define PROCESS_STACK_VA_TOP   0x00030FFF
#define MAX_PROCESS_COUNT      20

static ProcessInfo processTable[MAX_PROCESS_COUNT];
static UINT processCount;

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes)
{
    FUNC_ENTRY ("Process start address: 0x%px, size: 0x%x bytes", processStartAddress,
                binLengthBytes);

    // Process table entry
    if (processCount == MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, KERNEL_EXIT_FAILURE);
    }

    ProcessInfo* pinfo = &processTable[processCount++];
    pinfo->state       = PROCESS_NOT_CREATED;

    // Allocate physical memory for new PD
    if (!kpg_createNewPageDirectory (&pinfo->pagedir, PG_NEWPD_FLAG_COPY_KERNEL_PAGES |
                                                          PG_NEWPD_FLAG_RECURSIVE_MAP)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Cannot create new process.
    }

    // Copy the program to a page aligned physical address
    if (kpmm_alloc (&pinfo->bin_addr, 1, PMM_REGION_ANY) == false) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Physical memory allocation failed.
    }

    void* bin_va = kpg_temporaryMap (pinfo->bin_addr);
    k_memcpy (bin_va, processStartAddress, binLengthBytes);
    kpg_temporaryUnmap();

    pinfo->state = PROCESS_NOT_STARTED;
    return KERNEL_EXIT_SUCCESS; // Success
}

bool kprocess_switch (INT processID)
{
    FUNC_ENTRY ("Process ID: %u", processID);

    if (processCount >= MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_INVALID_RANGE, false);
    }

    ProcessInfo* pinfo = &processTable[processID];
    if (!kpg_switchPageDirectory (pinfo->pagedir)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Switch to new PD failed.
    }

    if (pinfo->state == PROCESS_NOT_STARTED) {
        // Allocate physical storage for process stack
        if (!kpmm_alloc (&pinfo->stack_addr, 1, PMM_REGION_ANY)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // allocation failed
        }

        // Map binary and stack into the process address space
        PageDirectory pd = kpg_getcurrentpd();

        if (!kpg_map (pd, PROCESS_TEXT_VA_START, pinfo->bin_addr,
                      PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
        }

        if (!kpg_map (pd, PROCESS_STACK_VA_START, pinfo->stack_addr,
                      PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
        }
    }

    pinfo->state = PROCESS_STARTED;
    jump_to_usermode (GDT_SELECTOR_UDATA, GDT_SELECTOR_UCODE, (void*)PROCESS_STACK_VA_TOP,
                      (void (*)())PROCESS_TEXT_VA_START);

    NORETURN();
}
