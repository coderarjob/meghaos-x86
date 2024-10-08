/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process Management
 * ---------------------------------------------------------------------------
 */

#include <kassert.h>
#include <kdebug.h>
#include <stdbool.h>
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
#include <utils.h>
#include <x86/cpu.h>
#include <intrusive_list.h>
#include <intrusive_queue.h>
#include <vmm.h>
#include <memloc.h>

#define PROCESS_STACK_SIZE_PAGES 0x1
#define PROCESS_STACK_VA_TOP(stackstart, pages) \
    ((stackstart) + (pages)*CONFIG_PAGE_FRAME_SIZE_BYTES - 1)
#define MAX_PROCESS_COUNT 20

static UINT processCount;
static ProcessInfo* currentProcess = NULL;
static ListNode schedulerQueueHead = { 0 };

static bool s_switchProcess (ProcessInfo* nextProcess, ProcessRegisterState* currentProcessState);
static ProcessInfo* s_processInfo_malloc();
static ProcessInfo* s_dequeue();
static bool s_enqueue (ProcessInfo* p);
static bool s_createProcessPageDirectory (ProcessInfo* pinfo);
static bool s_setupProcessBinaryMemory (void* processStartAddress, SIZE binLengthBytes,
                                        ProcessInfo* pinfo);
static bool s_setupProcessStackMemory (ProcessInfo* pinfo);
static bool kprocess_kill_process (ProcessInfo** process);
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
static void s_showQueueItems (ListNode* forward, bool directionForward);
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
        "push [edx + proc_eflags];" // Restore eflags for kernel process
        "popfd;"
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

static ProcessInfo* s_processInfo_malloc (ProcessFlags flags)
{
    ProcessInfo* pInfo = kmalloc (sizeof (ProcessInfo));

    if (pInfo == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    if ((pInfo->registerStates = kmalloc (sizeof (ProcessRegisterState))) == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    pInfo->state     = PROCESS_STATE_INVALID;
    pInfo->processID = ++processCount; // First process have process ID = 1. 0 is Kernel.
    pInfo->flags     = flags;
    list_init (&pInfo->schedulerQueueNode);

    return pInfo;
}

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
static void s_showQueueItems (ListNode* forward, bool directionForward)
{
    ListNode* node;
    INFO ("Going %s:",
          (directionForward) ? "from forward to backward" : "from backward to forward");

    if (directionForward == true) {
        queue_for_each (forward, node)
        {
            ProcessInfo* q = LIST_ITEM (node, ProcessInfo, schedulerQueueNode);
            INFO ("%u", q->processID);
        }
    } else {
        queue_for_each_backward (forward, node)
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
    s_showQueueItems (&schedulerQueueHead, false);
#endif // DEBUG

    ListNode* node = dequeue (&schedulerQueueHead);
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

    enqueue (&schedulerQueueHead, &p->schedulerQueueNode);
    return true;
}

static bool s_createProcessPageDirectory (ProcessInfo* pinfo)
{
    FUNC_ENTRY ("Pinfo: %px", pinfo);

    if (BIT_ISUNSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        // Create physical memory for Page directory of the new non-thread process
        Physical newPD;
        if (!kpg_createNewPageDirectory (&newPD, PG_NEWPD_FLAG_COPY_KERNEL_PAGES |
                                                     PG_NEWPD_FLAG_RECURSIVE_MAP)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false); // Cannot create new process.
        }

        INFO ("New VMManager is being created");
        if (!(pinfo->context = kvmm_new (ARCH_MEM_START_PROCESS_MEMORY, ARCH_MEM_END_PROCESS_MEMORY,
                                         newPD, PMM_REGION_ANY))) {
            // Since the PD created before is yet not part of the 'context' we have to delete it
            // here as no one knows about it.
            if (!kpg_deletePageDirectory (newPD, PG_DELPD_FLAG_KEEP_KERNEL_PAGES)) {
                BUG(); // Cannot fail under normal operation. It was allocated just above.
            }
            RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
        }
    } else {
        // For thread type processes, we use the PD & VMM of its parent
        pinfo->context = kprocess_getCurrentContext();
    }

    return true;
}

static bool s_setupProcessBinaryMemory (void* processStartAddress, SIZE binLengthBytes,
                                        ProcessInfo* pinfo)
{
    FUNC_ENTRY ("processStartAddress: %px, binLengthBytes: %x, Pinfo: %px", processStartAddress,
                binLengthBytes, pinfo);

    // For Threads there is nothing to be setup for program text other thatn just setting of process
    // data.
    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        pinfo->binary.virtualMemoryStart = (PTR)processStartAddress;
        return true;
    }

    k_assert (pinfo != NULL && binLengthBytes > 0, "Invalid program size");

    pinfo->binary.virtualMemoryStart = ARCH_MEM_START_PROCESS_TEXT;
    pinfo->binary.sizePages          = BYTES_TO_PAGEFRAMES_CEILING (binLengthBytes);

    // Allocate virtual and physical memory for the program binary.
    Physical pa;
    if (!(kvmm_memmap (pinfo->context, pinfo->binary.virtualMemoryStart, NULL,
                       pinfo->binary.sizePages, VMM_MEMMAP_FLAG_IMMCOMMIT, &pa))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false); // allocation failed
    }
    // memmap stores some metadata (for debugging) about the process and purpose. PID in this case
    // needs to set, since the process calling the memmap is not the process that will use the
    // address space. Purpose is also one of the optional metadata. These have no effect for
    // non-debug builds.
    kvmm_setAddressSpaceMetadata (pinfo->context, pinfo->binary.virtualMemoryStart, "proc bin",
                                  &pinfo->processID);

    k_memcpyToPhyMem (pa, (PTR)processStartAddress, binLengthBytes);
    return true;
}

static bool s_setupProcessStackMemory (ProcessInfo* pinfo)
{
    FUNC_ENTRY ("Pinfo: %px", pinfo);

    // Process stacks are always allocated dynamically. Their sizes are fixed for now though
    VMemoryMemMapFlags flags = VMM_MEMMAP_FLAG_NONE;
    pinfo->stack.sizePages   = BYTES_TO_PAGEFRAMES_CEILING (ARCH_MEM_LEN_BYTES_PROCESS_STACK);

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        flags |= VMM_MEMMAP_FLAG_KERNEL_PAGE;
        // We need to pre-allocate & map physical memory for Kernel threads/processes. The reason is
        // this: Normally Physical pages are allocated in the page fault handler after a page fault.
        // After a page fault, control is passed to the Kernel (Ring 0 stack and privilage level
        // switch) and the page fault handler is called. It is here a physical page is allocated,
        // mapped to the faulting virtual address and control is passed back to the faulting
        // instruction. This however does not work for Kernel stack memory, and we get a tripple
        // fault. This is because the control is already in Ring 0 and when CPU calls interrupt
        // handler which pushes registers values causing another page fault.
        //
        // So I think we have to premap stack memory for Kernel threads and processes.
        flags |= VMM_MEMMAP_FLAG_IMMCOMMIT;
    }

    // Find virtual address for process stack
    PTR stackVA = kvmm_findFree (pinfo->context, pinfo->stack.sizePages + 2);
    if (stackVA == 0) {
        RETURN_ERROR (ERR_OUT_OF_MEM, false);
    }

    // NULL page at the bottom of the stack
    if (!kvmm_memmap (pinfo->context, stackVA, NULL, 1, VMM_MEMMAP_FLAG_NULLPAGE, NULL)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    // memmap stores some metadata (for debugging) about the process and purpose. PID in this case
    // needs to set, since the process calling the memmap is not the process that will use the
    // address space. Purpose is also one of the optional metadata. These have no effect for
    // non-debug builds.
    kvmm_setAddressSpaceMetadata (pinfo->context, stackVA, NULL, &pinfo->processID);

    // Stack itself
    stackVA += CONFIG_PAGE_FRAME_SIZE_BYTES;
    pinfo->stack.virtualMemoryStart = stackVA;
    if (!kvmm_memmap (pinfo->context, pinfo->stack.virtualMemoryStart, NULL, pinfo->stack.sizePages,
                      flags, NULL)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    // memmap stores some metadata (for debugging) about the process and purpose. PID in this case
    // needs to set, since the process calling the memmap is not the process that will use the
    // address space. Purpose is also one of the optional metadata. These have no effect for
    // non-debug builds.
    kvmm_setAddressSpaceMetadata (pinfo->context, stackVA, "proc stack", &pinfo->processID);

    // NULL page at the top of the stack
    stackVA += (pinfo->stack.sizePages * CONFIG_PAGE_FRAME_SIZE_BYTES);
    if (!kvmm_memmap (pinfo->context, stackVA, NULL, 1, VMM_MEMMAP_FLAG_NULLPAGE, NULL)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    // memmap stores some metadata (for debugging) about the process and purpose. PID in this case
    // needs to set, since the process calling the memmap is not the process that will use the
    // address space. Purpose is also one of the optional metadata. These have no effect for
    // non-debug builds.
    kvmm_setAddressSpaceMetadata (pinfo->context, stackVA, NULL, &pinfo->processID);

    return true;
}

static bool s_switchProcess (ProcessInfo* nextProcess, ProcessRegisterState* currentProcessState)
{
    FUNC_ENTRY ("ProcessInfo: %px, Current ProcessRegisterState: %px", nextProcess,
                currentProcessState);

    if (currentProcess != NULL) {
        INFO ("Current PID: %u, Next PID: %u", currentProcess->processID, nextProcess->processID);

        k_assert (currentProcessState != NULL,
                  "Current process state is invalid for current process");

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
    cr3.physical         = PHYSICAL_TO_PAGEFRAME (kvmm_getPageDirectory (nextProcess->context).val);

    // Physical memory for the Page Directory must be allocated.
    k_assert (kpmm_getPageStatus (kvmm_getPageDirectory (nextProcess->context)) == PMM_STATE_USED,
              "Process context invalid");

    ProcessRegisterState* reg = nextProcess->registerStates;

    INFO ("Is context switch required: Yes");
    INFO ("Kernel process: %x", BIT_ISSET (nextProcess->flags, PROCESS_FLAGS_KERNEL_PROCESS));
    INFO ("Process (PID: %u) starting. ss:esp =  %x:%x, cs:eip = %x:%x, eflags: %x",
          nextProcess->processID, reg->ds, reg->esp, reg->cs, reg->eip, reg->eflags);

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
static bool kprocess_kill_process (ProcessInfo** process)
{
    FUNC_ENTRY();

    if (processCount <= 1) {
        // Cannot exit when there is zero/single process running.
        RETURN_ERROR (ERR_PROC_EXIT_NOT_ALLOWED, false);
    }

    ProcessInfo* l_process = *process;

    k_assert (process != NULL, "There are no process to exit");

    INFO ("Killing process: %u", l_process->processID);
    INFO ("Is thread: %s", BIT_ISSET (l_process->flags, PROCESS_FLAGS_THREAD) ? "Yes" : "No");

    if (!kfree (l_process->registerStates)) {
        BUG(); // Cannot fail under normal operation. It was allocated so should also be freed.
    }

    // Delete complete context only for non-thread processes
    if (BIT_ISUNSET (l_process->flags, PROCESS_FLAGS_THREAD)) {
        INFO ("Removing complete process context");

        // If there is no current process or current process is not the process being killed, there
        // is no need to change CR3.
        if (currentProcess != NULL && currentProcess->processID == l_process->processID) {
            // In order to destroy the context of the current process, it is required to switch to
            // the Kernel context, otherwise we would be killing the context while using it.
            register x86_CR3 cr3 = { 0 };
            cr3.pcd              = x86_PG_DEFAULT_IS_CACHING_DISABLED;
            cr3.pwt              = x86_PG_DEFAULT_IS_WRITE_THROUGH;
            cr3.physical = PHYSICAL_TO_PAGEFRAME (kvmm_getPageDirectory (g_kstate.context).val);

            x86_LOAD_REG (CR3, cr3);
        }

        // VMM delete will free all the page tables and the VMemoryManager itself. Page Directory is
        // not deleted, which need to be done following VMM delete. Since we cannot acceess the PD
        // once vmm is deleted, we store it in a variable for later use.
        Physical pd = kvmm_getPageDirectory (l_process->context);

        // Delete the VMM and deallocate physical memory used by page tables
        if (!kvmm_delete (&l_process->context)) {
            BUG(); // Cannot fail under normal operation. It was allocated so should also be freed.
        }

        // Iterate through each of the PDEs and free physical memory for page tables as well.
        if (!kpg_deletePageDirectory (pd, PG_DELPD_FLAG_KEEP_KERNEL_PAGES)) {
            BUG(); // Cannot fail under normal operation. It was allocated so should also be freed.
        }
    } else {
        INFO ("Removing thread context");
        if (!kvmm_free (l_process->context, currentProcess->stack.virtualMemoryStart)) {
            BUG(); // Cannot fail under normal operation. It was allocated so should also be freed.
        }
    }

    queue_remove (&l_process->schedulerQueueNode);
    kfree (l_process);
    *process = NULL;
    processCount--;

    return true;
}
#pragma GCC diagnostic pop

void kprocess_init()
{
    list_init (&schedulerQueueHead);
}

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags)
{
    FUNC_ENTRY ("Process start address: %px, size: %x bytes, flags: %x", processStartAddress,
                binLengthBytes, flags);

    if (processCount == MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, KERNEL_EXIT_FAILURE);
    }

    ProcessInfo* pinfo = s_processInfo_malloc (flags);
    INFO ("Creating new process: ID = %u", pinfo->processID);

    if (!s_createProcessPageDirectory (pinfo)) {
        // On failure the process page directory is deleted by s_createProcessPageDirectory itself
        // and so there is nothing to be done here but to exit.
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    if (!s_setupProcessBinaryMemory (processStartAddress, binLengthBytes, pinfo)) {
        goto failure;
    }

    if (!s_setupProcessStackMemory (pinfo)) {
        goto failure;
    }

    INFO ("New process: ID = %u", pinfo->processID);
    kvmm_printVASList (pinfo->context);
    INFO ("------------------------");

    //  Setup register states
    ProcessRegisterState* regs = pinfo->registerStates;
    regs->ebx                  = 0;
    regs->esi                  = 0;
    regs->edi                  = 0;
    regs->eflags               = X86_EFLAGS_INTERRUPT_ENABLE | X86_EFLAGS_BIT1_ALWAYS_ONE;
    regs->cs                   = GDT_SELECTOR_UCODE;
    regs->ds                   = GDT_SELECTOR_UDATA;
    regs->ebp                  = 0; // This is required for stack trace to end.
    regs->esp = PROCESS_STACK_VA_TOP ((U32)pinfo->stack.virtualMemoryStart, pinfo->stack.sizePages);
    regs->eip = (U32)pinfo->binary.virtualMemoryStart;

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        regs->ds = GDT_SELECTOR_KDATA;
        regs->cs = GDT_SELECTOR_KCODE;
    }

    pinfo->state = PROCESS_STATE_IDLE;

    if (!s_enqueue (pinfo)) {
        goto failure;
    }

    INFO ("Process with ID %u created.", pinfo->processID);
    return (INT)pinfo->processID;

failure:
    kprocess_kill_process (&pinfo);
    RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
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

    // clang-format off
    // In a Kernel process, no stack switch happens in a system call and the same process stack is
    // used when within kernel as well. So before performing the following actions the stack needs
    // to not use the process stack.
    // 1. When killing kernel thread, the stack of the thread needs to be deallocated.
    // 2. When killing kernel process, the whole address space is deallocated.
    __asm__ volatile(
                    "   push ebp;"
                    "   mov ebp, esp;"
                    "   test %1, PROCESS_FLAGS_KERNEL_PROCESS;"
                    "   jz .cont;"
                    //////////////////////////////////////////////////////////////////////////////
                    // We temporarily change to use the Kernel stack before starting the actual
                    // exit process. Two things can happen if the kprocess_exit succeeds then the
                    // current process will be killed and this stack state will end after yielding
                    // to the next process. If kprocess_exit fails, it will return to this routine
                    // and the original stack pointer will be restored. NOTE:Using of Kernel stack
                    // in another process works because whole of the Kernel address space is mapped
                    // to each process. NOTE: As we have switched stack, local variables/function
                    // arguments of this function which were on the stack cannot be accesssed from
                    // here on.
                    "   mov esp, " STR (MEM_KSTACK_TOP) ";"
                    //////////////////////////////////////////////////////////////////////////////
                    ".cont:;"
                     //////////////////////////////////////////////////////////////////////////////
                    // Call to kprocess_kill_process to kill 'currentProcess'.
                    "   lea eax, [currentProcess];"
                    "   push eax;"
                    "   call kprocess_kill_process;"
                    "   add esp, 4;"
                     //////////////////////////////////////////////////////////////////////////////
                    // In case kprocess_kill_process fails, there is nothing to do but exit.
                    "   cmp eax, 0;"
                    "   jz .fin;"
                     //////////////////////////////////////////////////////////////////////////////
                    // Call to kprocess_yield to go to the next process now that the current one is
                    // destoryed.
                    "   xor eax, eax;"      //  NULL is passed in the first argument of yeld.
                    "   push eax;"
                    "   call kprocess_yield;"
                    "   add esp, 4;"
                     //////////////////////////////////////////////////////////////////////////////
                    // On success yield either returns either true or jumps to the next process,
                    // false otherwise. But here we simply return anything it is returning.
                    "   mov %0, eax;"
                    ".fin:;"
                    "   mov esp, ebp;"
                    "   pop ebp;"
                    //////////////////////////////////////////////////////////////////////////////
                    : "=r"(ret)
                    : "r"(flags)
                    : // No clobber
    );
    // clang-format on

    return (ret == true);
}

VMemoryManager* kprocess_getCurrentContext()
{
    return (currentProcess == NULL) ? g_kstate.context : currentProcess->context;
}

UINT kprocess_getCurrentPID()
{
    return (currentProcess == NULL) ? PROCESS_ID_KERNEL : currentProcess->processID;
}
