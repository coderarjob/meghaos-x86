/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process Management
 * ---------------------------------------------------------------------------
 */

#include <kassert.h>
#include <kdebug.h>
#include <types.h>
#include <pmm.h>
#include <kerror.h>
#include <paging.h>
#include <kstdlib.h>
#include <process.h>
#include <x86/process.h>
#include <x86/gdt.h>
#include <memmanage.h>
#include <x86/memloc.h>
#include <utils.h>

#define PROCESS_TEXT_VA_START  0x00010000
#define PROCESS_STACK_VA_START 0x00030000
#define PROCESS_STACK_VA_TOP   0x00030FFF
#define MAX_PROCESS_COUNT      20

static ProcessInfo* processTable[MAX_PROCESS_COUNT];
static UINT processCount;
static ProcessInfo* currentProcess = NULL;

static ProcessInfo* s_processInfo_malloc();

static ProcessInfo* s_processInfo_malloc()
{
    if (processCount == MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, NULL);
    }

    ProcessInfo* pInfo = kmalloc (sizeof (ProcessInfo));

    if (pInfo == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    if ((pInfo->registerStates = kmalloc (sizeof (ProcessRegisterState))) == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    pInfo->state = PROCESS_NOT_CREATED;

    // Find a hole for the new process in the process table.
    int pid = 0;
    for (pid = 0; pid < MAX_PROCESS_COUNT; pid++) {
        if (processTable[pid] == NULL) {
            break;
        }
    }
    pInfo->processID  = pid;
    processTable[pid] = pInfo;
    processCount++;

    return pInfo;
}

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags)
{
    FUNC_ENTRY ("Process start address: 0x%px, size: 0x%x bytes", processStartAddress,
                binLengthBytes);

    // Process table entry
    ProcessInfo* pinfo = s_processInfo_malloc();
    if (pinfo == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    if (BIT_ISUNSET (flags, PROCESS_FLAGS_THREAD)) {
        // Allocate physical memory for new PD
        if (!kpg_createNewPageDirectory (&pinfo->pagedir, PG_NEWPD_FLAG_COPY_KERNEL_PAGES |
                                                              PG_NEWPD_FLAG_RECURSIVE_MAP)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Cannot create new process.
        }

        // Copy the program to a page aligned physical address
        if (kpmm_alloc (&pinfo->binaryAddress, 1, PMM_REGION_ANY) == false) {
            RETURN_ERROR (ERROR_PASSTHROUGH,
                          KERNEL_EXIT_FAILURE); // Physical memory allocation failed.
        }

        void* bin_va = kpg_temporaryMap (pinfo->binaryAddress);
        k_memcpy (bin_va, processStartAddress, binLengthBytes);
        kpg_temporaryUnmap();
    }

    pinfo->state               = PROCESS_NOT_STARTED;
    pinfo->flags               = flags;
    pinfo->registerStates->ds  = GDT_SELECTOR_UDATA;
    pinfo->registerStates->cs  = GDT_SELECTOR_UCODE;
    pinfo->registerStates->eip = PROCESS_TEXT_VA_START;
    pinfo->registerStates->esp = PROCESS_STACK_VA_TOP;

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        pinfo->registerStates->eip = (PTR)processStartAddress;
    }

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        pinfo->registerStates->ds = GDT_SELECTOR_KDATA;
        pinfo->registerStates->cs = GDT_SELECTOR_KCODE;
    }
    return pinfo->processID;
}

void kprocess_setCurrentProcessRegisterStates (ProcessRegisterState state)
{
    k_assert (currentProcess != NULL, "There are no process running.");
    k_memcpy (currentProcess->registerStates, &state, sizeof (ProcessRegisterState));
}

bool kprocess_switch (UINT processID)
{
    FUNC_ENTRY ("Process ID: %u", processID);

    if (processID >= processCount) {
        RETURN_ERROR (ERR_INVALID_RANGE, false);
    }

    ProcessInfo* pinfo = processTable[processID];

    // Threads use the PD of its parent.
    if (BIT_ISUNSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        if (!kpg_switchPageDirectory (pinfo->pagedir)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Switch to new PD failed.
        }
    }

    if (pinfo->state == PROCESS_NOT_STARTED) {
        PageDirectory pd = kpg_getcurrentpd();

        if (BIT_ISUNSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
            // Map process binary location into the process's address space
            if (!kpg_map (pd, PROCESS_TEXT_VA_START, pinfo->binaryAddress,
                          PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
                RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
            }
        }

        // Allocate physical storage for process stack
        if (!kpmm_alloc (&pinfo->stackAddress, 1, PMM_REGION_ANY)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // allocation failed
        }

        // Map process stack location into the process's address space
        U32 map_flags = PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED;
        if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
            map_flags |= PG_MAP_FLAG_KERNEL;
        }

        if (!kpg_map (pd, PROCESS_STACK_VA_START, pinfo->stackAddress, map_flags)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
        }
    }

    pinfo->state   = PROCESS_STARTED;
    currentProcess = pinfo;

    INFO ("Kernel process: 0x%x", BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS));

    ProcessRegisterState* reg = pinfo->registerStates;

    INFO ("Process (PID: %u) starting. ss:esp =  0x%x:0x%x, cs:eip = 0x%x:0x%x", processID, reg->ds,
          reg->esp, reg->cs, reg->eip);

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        jump_to_kernelprocess ((void*)reg->esp, (void (*)())reg->eip);
    } else {
        jump_to_userprocess (reg->ds, reg->cs, (void*)reg->esp, (void (*)())reg->eip);
    }

    NORETURN();
}
