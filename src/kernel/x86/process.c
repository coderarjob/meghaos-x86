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

#define PROCESS_TEXT_VA_START    0x00010000
#define PROCESS_STACK_VA_START   0x00030000
#define PROCESS_STACK_SIZE_PAGES 0x1
#define PROCESS_STACK_VA_TOP(stackstart, pages) \
    ((stackstart) + (pages)*CONFIG_PAGE_FRAME_SIZE_BYTES - 1)
#define PROCESS_TEMPORARY_MAP LINEAR_ADDR (KERNEL_PDE_INDEX - 1, TEMPORARY_PTE_INDEX, 0)
#define MAX_PROCESS_COUNT     20

static ProcessInfo* processTable[MAX_PROCESS_COUNT];
static UINT processCount;
static ProcessInfo* currentProcess = NULL;

static void s_temporaryUnmap();
static void* s_temporaryMap (Physical p);
static bool kprocess_switch (ProcessInfo* nextProcess, ProcessRegisterState* currentProcessState);

__attribute__ ((noreturn)) void jump_to_process (U32 type, x86_CR3 cr3, ProcessRegisterState* regs);

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
__asm__(".struct 0;"
        "   proc_ebx:    .struct . + 4;"
        "   proc_esi:    .struct . + 4;"
        "   proc_edi:    .struct . + 4;"
        "   proc_esp:    .struct . + 4;"
        "   proc_ebp:    .struct . + 4;"
        "   proc_eip:    .struct . + 4;"
        "   proc_eflags: .struct . + 4;"
        "   proc_cs:     .struct . + 4;"
        "   proc_ds:     .struct . + 4;"
        " proc_register_state_struct_size: .struct .;"
        //////////////////////////////////////////////////
        ".text;"
        ".global jump_to_process;"
        "jump_to_process:;"
        "mov ebp, esp;"
        "mov eax, [ebp + 4];"  // Process flags
        "mov ecx, [ebp + 8];"  // CR3 value.
        "mov edx, [ebp + 12];" // Base of ProcessRegisterState struct
        /////// Restore GP registers ////////
        "mov ebx, [edx + proc_ebx];" // General purpose registers
        "mov esi, [edx + proc_esi];" // General purpose registers
        "mov edi, [edx + proc_edi];" // General purpose registers
        "mov ebp, [edx + proc_ebp];" // Switch to process stack Base pointer
        /////// Segment registers ////////
        /// Segment registers are not preserved expect DS, SS and CS
        "mov ds, [edx + proc_ds];"
        "mov es, [edx + proc_ds];"
        "mov fs, [edx + proc_ds];"
        "mov gs, [edx + proc_ds];"
        /////// Change CR3 ////////
        // CR3 must change even for Threads because a thread can be scheduled to run after a process
        // which is not its parent.
        "mov cr3, ecx;"
        ///////////////////////////
        "test eax, PROCESS_FLAGS_KERNEL_PROCESS;"
        "jz .load_user_process;"
        /////// Load Kernel Process ////////
        "mov esp, [edx + proc_esp];" // Switch to process stack pointer
        "jmp [edx + proc_eip];"      // Kernel process entry/return address
        /////// Load User Process ////////
        ".load_user_process:;"
        /// Part of the functionality of IRET.
        "push ds;"                  // Stack segment selector
        "push [edx + proc_esp];"    // Process stack pointer
        "push [edx + proc_eflags];" // Process eflags
        "push [edx + proc_cs];"     // Code segment selector
        "push [edx + proc_eip];"    // User process entry/return address
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
    ProcessInfo* pInfo = kmalloc (sizeof (ProcessInfo));

    if (pInfo == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    if ((pInfo->registerStates = kmalloc (sizeof (ProcessRegisterState))) == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    pInfo->state     = PROCESS_NOT_CREATED;
    pInfo->processID = processCount++;

    return pInfo;
}

static INT s_queue_front = 0; // Points to where to dequeue next.
static INT s_queue_back  = 0; // Points to where to enqueue next.

static ProcessInfo* s_dequeue()
{
    if (s_queue_back == s_queue_front) {
        RETURN_ERROR (ERR_SCHEDULER_QUEUE_EMPTY, NULL);
    }

    ProcessInfo* pinfo          = processTable[s_queue_front];
    processTable[s_queue_front] = NULL;

    INT next      = (s_queue_front + 1) % MAX_PROCESS_COUNT;
    s_queue_front = next;
    return pinfo;
}

static bool s_enqueue (ProcessInfo* p)
{
    INT next = (s_queue_back + 1) % MAX_PROCESS_COUNT;
    if (next == s_queue_front) {
        RETURN_ERROR (ERR_SCHEDULER_QUEUE_FULL, false);
    }

    processTable[s_queue_back] = p;
    s_queue_back               = next;
    return true;
}

static bool s_setupProcessAddressSpace (ProcessInfo* pinfo)
{
    k_assert (pinfo != NULL && pinfo->state == PROCESS_NOT_CREATED, "Invalid state of process");
    k_assert (pinfo != NULL && pinfo->virt.Entry != 0, "Invalid address space requested");
    k_assert (pinfo != NULL && pinfo->virt.StackStart != 0, "Invalid address space requested");
    //  TODO: At this time stack of more than 1 page is not implemented.
    k_assert (pinfo != NULL && pinfo->physical.StackSizePages == 1, "Invalid stack size");

    PageDirectory pd = s_temporaryMap (pinfo->physical.PageDirectory);

    // Map process binary location into the process's address space
    if (BIT_ISUNSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        if (!kpg_map (pd, (PTR)pinfo->virt.Entry, pinfo->physical.Binary,
                      PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
        }
    }

    // Find virtual address for process stack
    // A thread runs in the same address space of its parent, this means that the
    // PROCESS_TEXT_VA_START will already be mapped and used by the root process. So it is
    // required that we find the next available address for the stack this this new thread
    // process.

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        // TODO: Think if the Region abstraction can solve cases like this.
        // TODO: The 'region_end' value is arbitrary and requires a proper solution.
        PTR stack_va_start = kpg_findVirtualAddressSpace (pd, pinfo->physical.StackSizePages,
                                                          PROCESS_STACK_VA_START,
                                                          PROCESS_STACK_VA_START + 0x5000);
        if (!stack_va_start) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false); // Could not find a address.
        }

        pinfo->virt.StackStart = stack_va_start;
    }

    // Map process stack location into the process's address space
    U32 map_flags = PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED;
    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        map_flags |= PG_MAP_FLAG_KERNEL;
    }

    if (!kpg_map (pd, (PTR)pinfo->virt.StackStart, pinfo->physical.Stack, map_flags)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false); // Map failed;
    }

    s_temporaryUnmap();
    return true;
}

static bool s_setupPhysicalMemoryForProcess (void* processStartAddress, SIZE binLengthBytes,
                                             ProcessFlags flags, ProcessInfo* pinfo)
{
    k_assert (pinfo != NULL && pinfo->state == PROCESS_NOT_CREATED, "Invalid state of process");
    k_assert (pinfo != NULL && pinfo->physical.StackSizePages > 0, "Invalid stack size");

    // Allocate physical storage for process stack
    if (!kpmm_alloc (&pinfo->physical.Stack, pinfo->physical.StackSizePages, PMM_REGION_ANY)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false); // allocation failed
    }

    // Create Physical memory for process/threads and copy the process binary into the space.
    if (BIT_ISUNSET (flags, PROCESS_FLAGS_THREAD)) {
        // Allocate physical memory for new Page Directory
        if (!kpg_createNewPageDirectory (&pinfo->physical.PageDirectory,
                                         PG_NEWPD_FLAG_COPY_KERNEL_PAGES |
                                             PG_NEWPD_FLAG_RECURSIVE_MAP)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false); // Cannot create new process.
        }

        // Allocate physical memory for the program binary.
        if (kpmm_alloc (&pinfo->physical.Binary, PROCESS_STACK_SIZE_PAGES, PMM_REGION_ANY) ==
            false) {
            // Physical memory allocation failed.
            RETURN_ERROR (ERROR_PASSTHROUGH, false);
        }

        // Copy the program to a page aligned physical address
        void* bin_va = kpg_temporaryMap (pinfo->physical.Binary);
        k_memcpy (bin_va, processStartAddress, binLengthBytes);
        kpg_temporaryUnmap();
    } else {
        x86_CR3 cr3 = { 0 };
        x86_READ_REG (CR3, cr3);
        pinfo->physical.PageDirectory.val = PAGEFRAME_TO_PHYSICAL (cr3.physical);
    }

    return true;
}

static bool kprocess_switch (ProcessInfo* nextProcess, ProcessRegisterState* currentProcessState)
{
    FUNC_ENTRY ("ProcessInfo: 0x%px, Current ProcessRegisterState: 0x%px", nextProcess,
                currentProcessState);

    if (currentProcess != NULL) {
        INFO ("Current PID: %u, Next PID: %u", currentProcess->processID, nextProcess->processID);

        if (currentProcess->processID == nextProcess->processID) {
            // If we are switching to the same process then no need to do any context switch, we
            // simply return from here and ultimately the system call will return to the process.
            // This can happen when there is only one process or when the current process is the
            // oldest process in the process table.
            INFO ("Is context switch required: No");
            return true;
        }

        currentProcess->state = PROCESS_IDLE;
        k_memcpy (currentProcess->registerStates, currentProcessState,
                  sizeof (ProcessRegisterState));
    }

    register x86_CR3 cr3 = { 0 };
    cr3.pcd              = x86_PG_DEFAULT_IS_CACHING_DISABLED;
    cr3.pwt              = x86_PG_DEFAULT_IS_WRITE_THROUGH;
    cr3.physical         = PHYSICAL_TO_PAGEFRAME (nextProcess->physical.PageDirectory.val);

    ProcessRegisterState* reg = nextProcess->registerStates;

    INFO ("Is context switch required: Yes");
    INFO ("Kernel process: 0x%x", BIT_ISSET (nextProcess->flags, PROCESS_FLAGS_KERNEL_PROCESS));
    INFO ("Process (PID: %u) starting. ss:esp =  0x%x:0x%x, cs:eip = 0x%x:0x%x",
          nextProcess->processID, reg->ds, reg->esp, reg->cs, reg->eip);

    nextProcess->state = PROCESS_RUNNING;
    currentProcess     = nextProcess;
    jump_to_process (nextProcess->flags, cr3, reg);

    NORETURN();
}

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags)
{
    FUNC_ENTRY ("Process start address: 0x%px, size: 0x%x bytes, flags: 0x%x", processStartAddress,
                binLengthBytes, flags);

    if (processCount == MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, KERNEL_EXIT_FAILURE);
    }

    ProcessInfo* pinfo = s_processInfo_malloc();

    pinfo->flags           = flags;
    pinfo->virt.Entry      = PROCESS_TEXT_VA_START;
    pinfo->virt.StackStart = PROCESS_STACK_VA_START;
    if (BIT_ISSET (flags, PROCESS_FLAGS_THREAD)) {
        pinfo->virt.Entry = (PTR)processStartAddress;
    }
    pinfo->physical.StackSizePages = PROCESS_STACK_SIZE_PAGES;

    if (!s_setupPhysicalMemoryForProcess (processStartAddress, binLengthBytes, flags, pinfo)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Cannot create new process.
    }

    if (!s_setupProcessAddressSpace (pinfo)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Cannot create new process.
    }

    //  Setup register states
    ProcessRegisterState* regs = pinfo->registerStates;
    regs->ebx                  = 0;
    regs->esi                  = 0;
    regs->edi                  = 0;
    regs->esp = PROCESS_STACK_VA_TOP ((U32)pinfo->virt.StackStart, pinfo->physical.StackSizePages);
    regs->ebp = 0; // This is required for stack trace to end.
    regs->eip = (U32)pinfo->virt.Entry;
    regs->eflags = 0;
    regs->cs     = GDT_SELECTOR_UCODE;
    regs->ds     = GDT_SELECTOR_UDATA;

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        regs->ds = GDT_SELECTOR_KDATA;
        regs->cs = GDT_SELECTOR_KCODE;
    }

    pinfo->state = PROCESS_NOT_STARTED;

    if (!s_enqueue (pinfo)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    INFO ("Process with ID %u created.", pinfo->processID);
    return (INT)pinfo->processID;
}

bool kprocess_yield (ProcessRegisterState* currentState)
{
    FUNC_ENTRY ("currentState: 0x%px", currentState);

    if (currentProcess != NULL) {
        k_memcpy (currentProcess->registerStates, currentState, sizeof (ProcessRegisterState));
    }

    // If there were only a single process, then the forward pointer will point to it. When this
    // process yields the next process to run will come out to be itself, because the forward
    // pointer is pointing to it. In this situation two yields are requried to get past the current
    // process.
    // So if there are more than one process in the process table, and dequeue returns the current
    // process, then we dequeue once again.
    ProcessInfo* pinfo = NULL;
    bool loop_again    = false;
    do {
        pinfo = s_dequeue();
        if (pinfo == NULL) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false);
        }

        if (!s_enqueue (pinfo)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false);
        }
        loop_again = (processCount > 1 && currentProcess &&
                      pinfo->processID == currentProcess->processID);
    } while (loop_again);

    INFO ("ProcessCount: %u, currentProcess: 0x%px, pinfo.processID: %u", processCount,
          currentProcess, pinfo->processID);

    return kprocess_switch (pinfo, currentState);
}
