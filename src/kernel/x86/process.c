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
#include <x86/paging.h>
#include <kstdlib.h>
#include <process.h>
#include <x86/process.h>
#include <x86/gdt.h>
#include <memmanage.h>
#include <x86/memloc.h>
#include <utils.h>
#include <x86/cpu.h>

#define PROCESS_TEXT_VA_START  0x00010000
#define PROCESS_STACK_VA_START 0x00030000
#define PROCESS_STACK_VA_TOP   0x00030FFF
#define PROCESS_TEMPORARY_MAP  LINEAR_ADDR (KERNEL_PDE_INDEX - 1, TEMPORARY_PTE_INDEX, 0)
#define MAX_PROCESS_COUNT      20

static ProcessInfo* processTable[MAX_PROCESS_COUNT];
static UINT processCount;
static ProcessInfo* currentProcess = NULL;

static ProcessInfo* s_processInfo_malloc();
static void s_temporaryUnmap();
static void* s_temporaryMap (Physical p);

/***************************************************************************************************
 * Jumps to the entry point of a process and switches stacks, address space depending on process
 * flags.
 *
 * @Input flags         Process flags
 * @Input stack pointer User mode stack top virtual address.
 * @Input user_func     Jumps to this function in the user mode code segment.
 * @Input dataselector  User mode Data segment selector. (unused for kernel process)
 * @Input codeselector  User mode Code segment selector. (unused for kernel process)
 * @Input cr3           Value to be loaded into the CR3 register. (unused for thread process)
 * @return              Does not return.
 **************************************************************************************************/
__asm__(".text;"
        ".global jump_to_process;"
        "jump_to_process:;"
        "mov ebp, esp;"
        "mov eax, [ebp + 4];"  // Process flags
        "mov ebx, [ebp + 8];"  // Stack pointer
        "mov ecx, [ebp + 12];" // Pointer to function to jump to.
        "mov edx, [ebp + 16];" // data selector. Ignored if kernel process
        "mov esi, [ebp + 20];" // code selector. Ignored if kernel process.
        "mov edi, [ebp + 24];" // CR3 value. Ignored if thread process.
        "test eax, PROCESS_FLAGS_THREAD;"
        "jnz .cont1;" // CR3 should not change for thread processes.
        /////// Change CR3 ////////
        "mov cr3, edi;"
        ///////////////////////////
        ".cont1:;"
        "test eax, PROCESS_FLAGS_KERNEL_PROCESS;"
        "jz .load_user_process;"
        /////// Load Kernel Process ////////
        "mov esp, ebx;"
        "xor ebp, ebp;" // Required for stack trace to work. Ends here.
        "jmp ecx;"
        /////// Load User Process ////////
        ".load_user_process:;"
        "mov ds, ax;"
        "mov es, ax;"
        "mov fs, ax;"
        "mov gs, ax;"
        "push eax;"     // Stack segment selector = DS
        "push ebp;"     //  User mode stack pointer
        "pushfd;"       //  EFLAGS
        "push esi;"     // Code Segment selector
        "push ecx;"     // Function pointer
        "xor ebp, ebp;" // Required for stack trace to work. Ends here.
        "iret;");

static void s_temporaryUnmap()
{
    if (!kpg_unmap (kpg_getcurrentpd(), (PTR)PROCESS_TEMPORARY_MAP)) {
        k_panicOnError();
    }
}

static void* s_temporaryMap (Physical p)
{
    if (!kpg_map (kpg_getcurrentpd(), (PTR)PROCESS_TEMPORARY_MAP, p,
                  PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_KERNEL | PG_MAP_FLAG_CACHE_ENABLED)) {
        k_panicOnError();
    }
    return (void*)PROCESS_TEMPORARY_MAP;
}

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
    } else {
        // Threads should use the same Page Directory as its parent
        x86_CR3 cr3 = { 0 };
        x86_READ_REG (CR3, cr3);
        pinfo->pagedir.val = PAGEFRAME_TO_PHYSICAL (cr3.physical);
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

    if (pinfo->state == PROCESS_NOT_STARTED) {
        PageDirectory pd = kpg_getcurrentpd();

        if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
            pd = s_temporaryMap (pinfo->pagedir);
        }

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

        PTR stack_va_start = PROCESS_STACK_VA_START;

        if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
            // A thread runs in the same address space of its parent, this means that the
            // PROCESS_TEXT_VA_START will already be mapped and used by the root process. So it is
            // required that we find the next available address for the stack this this new thread
            // process.
            // TODO: Think if the Region abstraction can solve cases like this.
            // TODO: The 'region_end' value is arbitrary and requires a proper solution.
            stack_va_start = kpg_findVirtualAddressSpace (pd, 1, PROCESS_STACK_VA_START,
                                                          PROCESS_STACK_VA_START + 0x5000);
            if (stack_va_start == 0) {
                k_panicOnError();
            }
        }

        if (!kpg_map (pd, stack_va_start, pinfo->stackAddress, map_flags)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
        }
        s_temporaryUnmap();
    }

    pinfo->state   = PROCESS_STARTED;
    currentProcess = pinfo;

    INFO ("Kernel process: 0x%x", BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS));

    ProcessRegisterState* reg = pinfo->registerStates;

    INFO ("Process (PID: %u) starting. ss:esp =  0x%x:0x%x, cs:eip = 0x%x:0x%x", processID, reg->ds,
          reg->esp, reg->cs, reg->eip);

    U32 cr3 = PHYSICAL_TO_PAGEFRAME (pinfo->pagedir.val);

    jump_to_process (pinfo->flags, (void*)reg->esp, (void (*)())reg->eip, reg->ds, reg->cs, cr3);

    NORETURN();
}
