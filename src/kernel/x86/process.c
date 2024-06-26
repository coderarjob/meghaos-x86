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
#include <intrusive_list.h>
#include <intrusive_queue.h>

#define PROCESS_TEXT_VA_START  0x00010000
#define PROCESS_STACK_VA_START 0x00030000

#define PROCESS_STACK_SIZE_PAGES 0x1
#define PROCESS_STACK_VA_TOP(stackstart, pages) \
    ((stackstart) + (pages)*CONFIG_PAGE_FRAME_SIZE_BYTES - 1)
#define MAX_PROCESS_COUNT 20

typedef struct SchedulerQueue {
    ListNode forward;
    ListNode backward;
} SchedulerQueue;

static UINT processCount;
static ProcessInfo* currentProcess   = NULL;
static SchedulerQueue schedulerQueue = { 0 };

static bool s_switchProcess (ProcessInfo* nextProcess, ProcessRegisterState* currentProcessState);
static ProcessInfo* s_processInfo_malloc();
static ProcessInfo* s_dequeue();
static bool s_enqueue (ProcessInfo* p);
static bool s_setupProcessAddressSpace (ProcessInfo* pinfo);
static bool s_setupPhysicalMemoryForProcess (void* processStartAddress, SIZE binLengthBytes,
                                             ProcessFlags flags, ProcessInfo* pinfo);
static bool kprocess_exit_internal();
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
static void s_showQueueItems (ListNode* forward, ListNode* backward, bool directionForward);
#endif // DEBUG

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

static ProcessInfo* s_processInfo_malloc()
{
    ProcessInfo* pInfo = kmalloc (sizeof (ProcessInfo));

    if (pInfo == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    if ((pInfo->registerStates = kmalloc (sizeof (ProcessRegisterState))) == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    pInfo->state     = PROCESS_STATE_INVALID;
    pInfo->processID = processCount++;
    list_init (&pInfo->schedulerQueueNode);

    return pInfo;
}

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
static void s_showQueueItems (ListNode* forward, ListNode* backward, bool directionForward)
{
    ListNode* node;
    INFO ("Going %s:",
          (directionForward) ? "from forward to backward" : "from backward to forward");

    if (directionForward == true) {
        queue_for_each_forward (forward, backward, node)
        {
            ProcessInfo* q = LIST_ITEM (node, ProcessInfo, schedulerQueueNode);
            INFO ("%u", q->processID);
        }
    } else {
        queue_for_each_backward (forward, backward, node)
        {
            ProcessInfo* q = LIST_ITEM (node, ProcessInfo, schedulerQueueNode);
            INFO ("%u", q->processID);
        }
    }
}
#endif // (DEBUG_LEVEL & 1) && !defined(UNITTEST)

static ProcessInfo* s_dequeue()
{
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
    s_showQueueItems (&schedulerQueue.forward, &schedulerQueue.backward, false);
#endif // DEBUG

    ListNode* node = dequeue_forward (&schedulerQueue.forward, &schedulerQueue.backward);
    if (node == NULL) {
        RETURN_ERROR (ERR_SCHEDULER_QUEUE_EMPTY, NULL);
    }

    ProcessInfo* pinfo = (ProcessInfo*)LIST_ITEM (node, ProcessInfo, schedulerQueueNode);
    return pinfo;
}

static bool s_enqueue (ProcessInfo* p)
{
    if (processCount >= MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_SCHEDULER_QUEUE_FULL, false);
    }

    enqueue_back (&schedulerQueue.backward, &p->schedulerQueueNode);
    return true;
}

static bool s_setupProcessAddressSpace (ProcessInfo* pinfo)
{
    k_assert (pinfo != NULL && pinfo->state == PROCESS_STATE_INVALID, "Invalid state of process");
    k_assert (pinfo != NULL && pinfo->virt.Entry != 0, "Invalid address space requested");
    k_assert (pinfo != NULL && pinfo->virt.StackStart != 0, "Invalid address space requested");
    //  TODO: At this time stack of more than 1 page is not implemented.
    k_assert (pinfo != NULL && pinfo->physical.StackSizePages == 1, "Invalid stack size");

    PageDirectory pd = kpg_temporaryMap (pinfo->physical.PageDirectory);

    // Map process binary location into the process's address space
    if (BIT_ISUNSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        //  TODO: At this time program of more than 1 page is not implemented.
        k_assert (pinfo != NULL && pinfo->physical.BinarySizePages == 1, "Invalid program size");

        if (!kpg_map (pd, (PTR)pinfo->virt.Entry, pinfo->physical.Binary,
                      PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false); // Map failed
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

    kpg_temporaryUnmap();
    return true;
}

static bool s_setupPhysicalMemoryForProcess (void* processStartAddress, SIZE binLengthBytes,
                                             ProcessFlags flags, ProcessInfo* pinfo)
{
    k_assert (pinfo != NULL && pinfo->state == PROCESS_STATE_INVALID, "Invalid state of process");
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
        k_assert (pinfo != NULL && pinfo->physical.BinarySizePages > 0, "Invalid program size");

        if (kpmm_alloc (&pinfo->physical.Binary, pinfo->physical.BinarySizePages, PMM_REGION_ANY) ==
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

static bool s_switchProcess (ProcessInfo* nextProcess, ProcessRegisterState* currentProcessState)
{
    FUNC_ENTRY ("ProcessInfo: %px, Current ProcessRegisterState: %px", nextProcess,
                currentProcessState);

    if (currentProcess != NULL) {
        k_assert (currentProcessState != NULL,
                  "Current process state is invalid for current process");
        INFO ("Current PID: %u, Next PID: %u", currentProcess->processID, nextProcess->processID);

        if (currentProcess->processID == nextProcess->processID) {
            // If we are switching to the same process then no need to do any context switch, we
            // simply return from here and ultimately the system call will return to the process.
            // This can happen when there is only one process or when the current process is the
            // oldest process in the process table.
            INFO ("Is context switch required: No");
            return true;
        }

        currentProcess->state = PROCESS_STATE_IDLE;
        k_memcpy (currentProcess->registerStates, currentProcessState,
                  sizeof (ProcessRegisterState));
    }

    register x86_CR3 cr3 = { 0 };
    cr3.pcd              = x86_PG_DEFAULT_IS_CACHING_DISABLED;
    cr3.pwt              = x86_PG_DEFAULT_IS_WRITE_THROUGH;
    cr3.physical         = PHYSICAL_TO_PAGEFRAME (nextProcess->physical.PageDirectory.val);

    // Physical memory for the Page Directory must be allocated.
    k_assert (kpmm_getPageStatus (nextProcess->physical.PageDirectory) == PMM_STATE_USED,
              "Process context invalid");

    ProcessRegisterState* reg = nextProcess->registerStates;

    INFO ("Is context switch required: Yes");
    INFO ("Kernel process: %x", BIT_ISSET (nextProcess->flags, PROCESS_FLAGS_KERNEL_PROCESS));
    INFO ("Process (PID: %u) starting. ss:esp =  %x:%x, cs:eip = %x:%x", nextProcess->processID,
          reg->ds, reg->esp, reg->cs, reg->eip);

    nextProcess->state = PROCESS_STATE_RUNNING;
    currentProcess     = nextProcess;
    jump_to_process (nextProcess->flags, cr3, reg);

    UNREACHABLE();
}

// TODO: There is no way to pass error codes/return codes to the parent process. Possible solution
// would to implement signals. When a process ending it would add SIGCHILD signal for its parent
// and the scheduler will make sure that the parent gets the message. However I do not want a ZOMBIE
// process as well.
// TODO: Ending a process should also end threads of the process.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static bool kprocess_exit_internal()
{
    FUNC_ENTRY();

    if (processCount < 2) {
        return false; // Cannot exit when there is zero/single process running.
    }

    k_assert (currentProcess != NULL, "There are no process to exit");

    INFO ("Killing process: %u", currentProcess->processID);
    INFO ("Is thread: %s", BIT_ISSET (currentProcess->flags, PROCESS_FLAGS_THREAD) ? "Yes" : "No");

    if (!kfree (currentProcess->registerStates)) {
        k_panicOnError();
    }

    // Delete complete context only for non-thread processes
    if (BIT_ISUNSET (currentProcess->flags, PROCESS_FLAGS_THREAD)) {
        INFO ("Removing complete process  context");

        if (!kpmm_free (currentProcess->physical.Binary,
                        currentProcess->physical.BinarySizePages)) {
            k_panicOnError();
        }

        // In order to destroy the context of the current process, it is required to switch to the
        // Kernel context, otherwise we would be killing the context while using it.
        register x86_CR3 cr3 = { 0 };
        cr3.pcd              = x86_PG_DEFAULT_IS_CACHING_DISABLED;
        cr3.pwt              = x86_PG_DEFAULT_IS_WRITE_THROUGH;
        cr3.physical         = PHYSICAL_TO_PAGEFRAME (g_page_dir.val);

        x86_LOAD_REG (CR3, cr3);

        // Iterate through each of the PDEs and free physical memory for page tables as well.
        if (!kpg_deletePageDirectory (currentProcess->physical.PageDirectory,
                                      PG_DELPD_FLAG_KEEP_KERNEL_PAGES)) {
            k_panicOnError();
        }
    } else {
        INFO ("Removing thread context");
        if (!kpg_unmap (kpg_getcurrentpd(), currentProcess->virt.StackStart)) {
            k_panicOnError();
        }
    }

    // Physical memory need to be freed explicitly
    if (!kpmm_free (currentProcess->physical.Stack, currentProcess->physical.StackSizePages)) {
        k_panicOnError();
    }

    queue_remove (&currentProcess->schedulerQueueNode);
    kfree (currentProcess);
    currentProcess = NULL;
    processCount--;

    // Now switch to the next process
    return kprocess_yield (NULL);
}
#pragma GCC diagnostic pop

void kprocess_init()
{
    queue_init (&schedulerQueue.forward, &schedulerQueue.backward);
}

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags)
{
    FUNC_ENTRY ("Process start address: %px, size: %x bytes, flags: %x", processStartAddress,
                binLengthBytes, flags);

    if (processCount == MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, KERNEL_EXIT_FAILURE);
    }

    ProcessInfo* pinfo = s_processInfo_malloc();

    INFO ("Creating new process: ID = %u", pinfo->processID);

    pinfo->flags           = flags;
    pinfo->virt.Entry      = PROCESS_TEXT_VA_START;
    pinfo->virt.StackStart = PROCESS_STACK_VA_START;
    if (BIT_ISSET (flags, PROCESS_FLAGS_THREAD)) {
        pinfo->virt.Entry = (PTR)processStartAddress;
    }
    pinfo->physical.StackSizePages  = PROCESS_STACK_SIZE_PAGES;
    pinfo->physical.BinarySizePages = BYTES_TO_PAGEFRAMES_CEILING (binLengthBytes);

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

    pinfo->state = PROCESS_STATE_IDLE;

    if (!s_enqueue (pinfo)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    INFO ("Process with ID %u created.", pinfo->processID);
    return (INT)pinfo->processID;
}

bool kprocess_yield (ProcessRegisterState* currentState)
{
    FUNC_ENTRY ("currentState: %px", currentState);

    // The scheduler should select processes in the "earliest idle process first" order. This is
    // ensured by the queue as well as the yield function.
    // NOTE:
    // If there were only a single process, then the forward pointer will point to it. When this
    // process yields the next process to run will come out to be itself, because the forward
    // pointer is pointing to it. In this situation two yields are required to get past the current
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

    k_assert (pinfo->state != PROCESS_STATE_INVALID, "Invalid process state");

    return s_switchProcess (pinfo, currentState);
}

bool kprocess_exit()
{
    UINT ret;
    UINT flags = (currentProcess != NULL) ? currentProcess->flags : 0;

    // In a Kernel process, no stack switch happens in a system call and the same process stack is
    // used when within kernel as well. So before performing the following actions the stack needs
    // to not use the process stack.
    // 1. When killing kernel thread, the stack of the thread needs to be deallocated.
    // 2. When killing kernel process, the whole address space is deallocated.
    __asm__ volatile("push ebp;"
                     "   mov ebp, esp;"
                     "   test %1, PROCESS_FLAGS_KERNEL_PROCESS;"
                     "   jz .cont;"
                     // We temporarily change to use the Kernel stack before starting the actual
                     // exit process. Two things can happen if the kprocess_exit succeeds then the
                     // current process will be killed and this stack state will end after yielding
                     // to the next process. If kprocess_exit fails, it will return to this routine
                     // and the original stack pointer will be restored. NOTE:Using of Kernel stack
                     // in another process works because whole of the Kernel address space is mapped
                     // to each process. NOTE: As we have switched stack, local variables/function
                     // arguments of this function which were on the stack cannot be accesssed from
                     // here on.
                     "   mov esp, " STR (INTEL_32_KSTACK_TOP) ";"
                    ".cont:;"
                    // Call to kprocess_exit will not
                    // return if it succeeds.
                    "   call kprocess_exit_internal;"
                    "   mov %0, eax;"
                    // kprocess_exit returned which means
                    // it did not succeed. Restore the
                    // stack.
                    "   mov esp, ebp;"
                    "   pop ebp;"
                     : "=r"(ret)
                     : "r"(flags)
                     : // No clobber
    );

    return (ret == true);
}
