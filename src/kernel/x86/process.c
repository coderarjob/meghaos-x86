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
#define PROCESS_STACK_VA_TOP(stackstart, pages) \
    ((stackstart) + (pages)*CONFIG_PAGE_FRAME_SIZE_BYTES - 1)
#define PROCESS_TEMPORARY_MAP LINEAR_ADDR (KERNEL_PDE_INDEX - 1, TEMPORARY_PTE_INDEX, 0)
#define MAX_PROCESS_COUNT     20

static ProcessInfo* processTable[MAX_PROCESS_COUNT];
static UINT processCount;
static ProcessInfo* currentProcess = NULL;

static void s_temporaryUnmap();
static void* s_temporaryMap (Physical p);
static bool kprocess_switch (ProcessInfo* pinfo);

void jump_to_process (U32 type, void* stackTop, void (*entry)(), U32 dataselector, U32 codeselector,
                      x86_CR3 cr3);

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
        "mov esp, ebx;"        // Use the stack of the process from here on.
        /////// Change CR3 ////////
        // CR3 must change even for Threads because a thread can be scheduled to run after a process
        // which is not its parent.
        "mov cr3, edi;"
        ///////////////////////////
        "test eax, PROCESS_FLAGS_KERNEL_PROCESS;"
        "jz .load_user_process;"
        /////// Load Kernel Process ////////
        "xor ebp, ebp;" // Required for stack trace to work. Ends here.
        "jmp ecx;"
        /////// Load User Process ////////
        ".load_user_process:;"
        "mov ds, edx;"
        "mov es, edx;"
        "mov fs, edx;"
        "mov gs, edx;"
        "push edx;"     // Data and Stack segment selector
        "push ebx;"     // User mode stack pointer
        "pushfd;"       // EFLAGS
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

    pInfo->state     = PROCESS_NOT_CREATED;
    pInfo->processID = processCount++;

    return pInfo;
}

static INT s_queue_front = 0; // Points to where to dequeue next.
static INT s_queue_back  = 0; // Points to where to enqueue next.

static ProcessInfo* s_dequeue()
{
    if (s_queue_back == s_queue_front) {
        RETURN_ERROR (ERR_INVALID_RANGE, NULL);
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
        RETURN_ERROR (ERR_OUT_OF_MEM, false);
    }

    processTable[s_queue_back] = p;
    s_queue_back               = next;
    return true;
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

    k_assert (processCount == 1 ||
                  (processCount > 1 && pinfo->processID != currentProcess->processID),
              "Queue returned the same process where there are others available.");
    return kprocess_switch (pinfo);
}

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags)
{
    FUNC_ENTRY ("Process start address: 0x%px, size: 0x%x bytes, flags: 0x%x", processStartAddress,
                binLengthBytes, flags);

    if (processCount == MAX_PROCESS_COUNT) {
        RETURN_ERROR (ERR_OUT_OF_MEM, KERNEL_EXIT_FAILURE);
    }

    ProcessInfo* pinfo = s_processInfo_malloc();

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

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_THREAD)) {
        pinfo->registerStates->eip = (PTR)processStartAddress;
    }

    if (BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS)) {
        pinfo->registerStates->ds = GDT_SELECTOR_KDATA;
        pinfo->registerStates->cs = GDT_SELECTOR_KCODE;
    }

    if (!s_enqueue (pinfo)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    INFO ("Process with ID %u created.", pinfo->processID);
    return (INT)pinfo->processID;
}

static bool kprocess_switch (ProcessInfo* pinfo)
{
    FUNC_ENTRY ("ProcessInfo: 0x%px", pinfo);

    if (pinfo->state == PROCESS_NOT_STARTED) {
        PageDirectory pd = s_temporaryMap (pinfo->pagedir);

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

        pinfo->registerStates->esp = PROCESS_STACK_VA_TOP(stack_va_start, 1);

        if (!kpg_map (pd, stack_va_start, pinfo->stackAddress, map_flags)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE); // Map failed
        }
        s_temporaryUnmap();
    }

    pinfo->state   = PROCESS_STARTED;
    currentProcess = pinfo;

    INFO ("Kernel process: 0x%x", BIT_ISSET (pinfo->flags, PROCESS_FLAGS_KERNEL_PROCESS));

    ProcessRegisterState* reg = pinfo->registerStates;

    INFO ("Process (PID: %u) starting. ss:esp =  0x%x:0x%x, cs:eip = 0x%x:0x%x", pinfo->processID,
          reg->ds, reg->esp, reg->cs, reg->eip);

    register x86_CR3 cr3 = { 0 };
    cr3.pcd              = x86_PG_DEFAULT_IS_CACHING_DISABLED;
    cr3.pwt              = x86_PG_DEFAULT_IS_WRITE_THROUGH;
    cr3.physical         = PHYSICAL_TO_PAGEFRAME (pinfo->pagedir.val);

    INFO ("Switching to process");
    jump_to_process (pinfo->flags, (void*)reg->esp, (void (*)())reg->eip, reg->ds, reg->cs, cr3);

    NORETURN();
}
