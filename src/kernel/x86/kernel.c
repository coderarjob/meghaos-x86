/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel 
*
* This is a test kernel to demostrate that the 'boot1' loader can actually load
* the kernel and jump to it.
* On successful execution, 'Kernel ready..' will be displayed on the top of the
* screen.
* ---------------------------------------------------------------------------
*
* Dated: 20th September 2020
*/

#include <kassert.h>
#include <disp.h>
#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>
#include <types.h>
#include <moslimits.h>
#include <kdebug.h>
#include <kerror.h>
#include <x86/interrupt.h>
#include <x86/vgatext.h>
#include <x86/tss.h>
#include <pmm.h>
#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/boot.h>
#include <x86/paging.h>
#include <panic.h>
#include <paging.h>
#include <utils.h>
#include <x86/memloc.h>
#include <memmanage.h>
#include <kstdlib.h>
#include <process.h>
#include <x86/cpu.h>
#include <kernel.h>
#include <vmm.h>
#include <drivers/x86/pc/8259_pic.h>
#include <drivers/x86/pc/8254_pit.h>
#ifdef GRAPHICS_MODE_ENABLED
    #include <graphics.h>
    #include <compositor.h>
#endif // GRAPHICS_MODE_ENABLED
#include <handle.h>

#define MPDEMO

static void display_system_info ();
static void s_initializeMemoryManagers ();
static SIZE s_getPhysicalBlockPageCount (Physical pa, Physical end);
static void run_root_process();

/* Kernel state global variable */
volatile KernelStateInfo g_kstate;

__attribute__ ((noreturn))
void kernel_main ()
{
    FUNC_ENTRY();

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_BOOT_COMPLETE);

#if defined(DEBUG) && !defined(GRAPHICS_MODE_ENABLED)
    // Initialize Text display
    kdisp_init();
#endif

    // Initilaize Physical Memory Manger
    kpmm_init ();

    display_system_info ();

    kearly_println ("[  ]\tVirtual memory management.");

    ksalloc_init();
    khandle_init();

    // Initialize VMM
    Physical kernelPD = HIGHER_HALF_KERNEL_TO_PA(MEM_START_KERNEL_PAGE_DIR);
    g_kstate.context = kvmm_new (MEM_START_KERNEL_LOW_REGION, MEM_END_KERNEL_HIGH_REGION,
                                   kernelPD, PMM_REGION_ANY);
    KERNEL_PHASE_SET (KERNEL_PHASE_STATE_VMM_READY);

    // Mark memory already occupied by the modules and unmap unused Virutal pages.
    s_initializeMemoryManagers();
    kvmm_printVASList(g_kstate.context);

    kearly_printf ("\r[OK]");

    // TSS setup
    kearly_println ("[  ]\tTSS setup.");
    ktss_init ();
    kearly_printf ("\r[OK]");

    // Usermode code segment
    kearly_println ("[  ]\tUser mode GDT setup.");
    kgdt_edit (GDT_INDEX_UCODE, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit (GDT_INDEX_UDATA, 0, 0xFFFFF, 0xF2, 0xD);
    kgdt_write ();
    kearly_printf ("\r[OK]");

    // Setup IDT
    kearly_println ("[  ]\tIDT setup");
    kidt_init ();

    kidt_edit (0, div_zero_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (8, double_fault_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (14, page_fault_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (13, general_protection_fault_asm_handler,GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (0x40, sys_dummy_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 3);
    kidt_edit (0x50, syscall_asm_despatcher, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 3);

#if MARCH == pc
    // External Maskable Interrupts are from Vector 0x20 to 0x30
    pic_init (0x20, 0x28);

    // Add handlers for timer and keyboard interrupts
    kidt_edit (0x20, timer_interrupt_asm_handler, GDT_SELECTOR_KCODE,
               IDT_DES_TYPE_32_INTERRUPT_GATE, 0);

    // Spurious IRQ handler (IRQ7 and IRQ15)
    kidt_edit (0x27, irq_7_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (0x2F, irq_15_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
#endif

    kearly_printf ("\r[OK]");

    kearly_println ("[  ]\tKernel memory management.");
    kmalloc_init();
    kearly_printf ("\r[OK]");

#ifdef GRAPHICS_MODE_ENABLED
    if (!graphics_init()) {
        ERROR ("Graphics mode could not be enabled");
    }
    kcompose_init();
#endif

    kearly_println ("[  ]\tProcess management & HW interrupts");
    kprocess_init();

    // Start timer receiving interupts
    pit_set_interrupt_counter(PIT_COUNTER_MODE_2,CONFIG_INTERRUPT_CLOCK_FREQ_HZ);
    pic_enable_disable_irq(PIC_IRQ_TIMER, true);
    ARCH_ENABLE_INTERRUPTS();

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_KERNEL_READY);
    kearly_printf ("\r[OK]");

#if defined(DEBUG) && !defined(GRAPHICS_MODE_ENABLED)
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,GREEN,0));
    kearly_println ("Kernel initialization finished..");
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,LIGHT_GRAY,0));
#endif

    //---------------
    int cpuflags_present = 0;
    __asm__ volatile("pushfd;"
                     "pushfd;"
                     "xor dword ptr [esp], (1 << 21);"
                     "popfd;"
                     "pushfd;"
                     "pop eax;"
                     "xor eax, [esp];"
                     "popfd;"
                     : "=a"(cpuflags_present));
    kearly_println ("CPUID detected: %x", cpuflags_present);

    int eax = 0;
    __asm__ volatile ("cpuid;":"+eax"(eax):"eax"(0));
    kearly_println ("CPUID [EAX=0]: %x", eax);

    run_root_process();
    k_halt();
}

void keventmanager_invoke()
{
    U64 us = KERNEL_TICK_COUNT_TO_MICROSEC (g_kstate.tick_count);

    if (us % CONFIG_PROCESS_PERIOD_US == 0) {
        UINT pid = kprocess_getCurrentPID();
        if (pid != PROCESS_ID_KERNEL) {
            INFO ("PID: %x, US: %u", pid, us);
            if (!kprocess_pushEvent (pid, KERNEL_EVENT_PROCCESS_YIELD_REQ, g_kstate.tick_count)) {
                BUG(); // Event push should not fail.
            }
        }
    }
#ifdef GRAPHICS_MODE_ENABLED
    if (us % CONFIG_PROCESS_PERIOD_US == 0) {
        kgraphis_flush();
    }
#endif
}

void k_delay (UINT ms)
{
    UINT us = ms * 1000;
    k_assert (us >= CONFIG_INTERRUPT_CLOCK_TP_MICROSEC, "Delay too small");

    U32 start_tick = g_kstate.tick_count;
    U32 end_tick   = start_tick + KERNEL_MICRODEC_TO_TICK_COUNT (us);

    while (g_kstate.tick_count < end_tick)
        ;
}

static void run_root_process()
{
    FUNC_ENTRY();

#ifdef GRAPHICS_MODE_ENABLED
    BootFileItem fileinfo = kboot_getBootFileItem (1);
#else
    #ifdef MPDEMO
    BootFileItem fileinfo = kboot_getBootFileItem (2);
    #else
    BootFileItem fileinfo = kboot_getBootFileItem (1);
    #endif // MPDEMO
#endif     // GRAPHICS_MODE_ENABLED

    Physical startAddress = PHYSICAL (fileinfo.startLocation);
    SIZE lengthBytes      = (SIZE)fileinfo.length;

    void* startAddress_va = HIGHER_HALF_KERNEL_TO_VA (startAddress);
    INT processID         = kprocess_create (startAddress_va, lengthBytes, PROCESS_FLAGS_NONE);
    if (processID < 0) {
        FATAL_BUG();
    }

    INFO ("Process ID: %u", processID);
    k_assert (processID == 1, "Root process must have process ID = 1");

    if (!kprocess_yield (NULL)) {
        FATAL_BUG();
    }

    UNREACHABLE();
    k_halt();
}

void display_system_info()
{
    FUNC_ENTRY();

#if defined(DEBUG) && defined(PORT_E9_ENABLED)
    INT loadedFilesCount = kboot_getBootFileItemCount();

    INFO ("Loaded kernel files:");
    for (INT i = 0; i < loadedFilesCount; i++) {
        BootFileItem file  = kboot_getBootFileItem (i);
        UINT startLocation = (UINT)file.startLocation;
        UINT length_bytes  = (UINT)file.length;

        INFO ("* file: Start = %x, Length = %u bytes", startLocation, length_bytes);
    }

    INT memoryMapItemCount = kboot_getBootMemoryMapItemCount();
    INFO ("BIOS Memory map:");
    U64 installed_memory = 0;
    for (INT i = 0; i < memoryMapItemCount; i++) {
        BootMemoryMapItem item  = kboot_getBootMemoryMapItem (i);
        U64 baseAddress         = item.baseAddr;
        U64 length_bytes        = item.length;
        BootMemoryMapTypes type = item.type;

        installed_memory += length_bytes;
        INFO ("* map: Start = %llx, Length = %llx, Type = %u", baseAddress, length_bytes, type);
    }

    INFO ("Kernel files loaded: %u", loadedFilesCount);
    INFO ("Max RAM Pages: %u", MAX_PAB_ADDRESSABLE_PAGE_COUNT);
    INFO ("Installed RAM bytes: %llx bytes", installed_memory);
    UINT installed_memory_pageCount = (UINT)BYTES_TO_PAGEFRAMES_CEILING (installed_memory);
    INFO ("Installed RAM Pages: %u", installed_memory_pageCount);
    INFO ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
#endif
}

/***************************************************************************************************
 * Returns the number of consecutive pages from 'pa' which have the same state in PAB as the state
 * of 'pa'. The search ends when physical memory address reaches 'end'.
 *
 * @return nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static SIZE s_getPhysicalBlockPageCount (Physical pa, Physical end)
{
    KernelPhysicalMemoryStates initialState = kpmm_getPageStatus (pa);
    KernelPhysicalMemoryStates state        = initialState;
    SIZE szPages                            = 0;

    while (initialState == state && pa.val < end.val) {
        pa.val += CONFIG_PAGE_FRAME_SIZE_BYTES;
        if ((state = kpmm_getPageStatus (pa)) == PMM_STATE_INVALID) {
            k_panicOnError();
        }
        szPages += 1;
    }

    return szPages;
}

/***************************************************************************************************
 * Completing the initialization up of PMM, VMM and page mappings based on BIOS memory map and
 * Module files information passed down from the Bootloader.
 *
 * After the basic initialization of PMM, here we allocate those pages which are specific to the
 * Kernel operation (Reserved memories of Kernel and area used by module files). After this is done,
 * the PMM is in a state that is expected by both the system and the Kernel.
 *
 * Now since there is 1:1 relattion between Physical memory and Virtual memory due the Higher Half
 * memory mapping, the VMM and page mappings need to be brought up to say the same thing as the PMM.
 *
 * @return nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static void s_initializeMemoryManagers()
{
    FUNC_ENTRY();

    // ---------------------------------------------------------------------------------------------
    // Kernel reserves a region at the very beginning of the physical memory. This is called the Low
    // Region.
    UINT pageCount = BYTES_TO_PAGEFRAMES_CEILING (MEM_LEN_BYTES_KERNEL_LOW_REGION);

    if (kpmm_allocAt (createPhysical (0), pageCount, PMM_REGION_ANY) == false) {
        k_panicOnError(); // Kernel low region of physical memory must be free.
    }

    // ---------------------------------------------------------------------------------------------
    // Then another reserved/used region is where the module files are loaded. PMM is made 'aware'
    // of that here.
    INT filesCount = kboot_getBootFileItemCount();

    BootFileItem firstFile      = kboot_getBootFileItem (0);
    BootFileItem lastFile       = kboot_getBootFileItem (filesCount - 1);
    Physical first_startAddress = PHYSICAL (firstFile.startLocation);

    Physical last_startAddress   = PHYSICAL (lastFile.startLocation);
    SIZE last_lengthBytes        = (USYSINT)lastFile.length;
    SIZE totalModulesLengthBytes = (last_startAddress.val - first_startAddress.val) +
                                   last_lengthBytes;

    INFO ("Total size of module files: %x bytes", totalModulesLengthBytes);

    UINT pageFrameCount = BYTES_TO_PAGEFRAMES_CEILING (totalModulesLengthBytes);
    if (kpmm_allocAt (first_startAddress, pageFrameCount, PMM_REGION_ANY) == false) {
        FATAL_BUG(); // Should not fail.
    }

    // ---------------------------------------------------------------------------------------------
    // Now that the PMM holds the complete picture of all the physical memories used & reserved, we
    // use that to initialize VMM and paging within the part which Higher Half mapped.
    Physical pa                = PHYSICAL (0);
    SIZE paRegionSizeBytes     = MIN (kpmm_getUsableMemorySize (PMM_REGION_ANY),
                                      HIGHER_HALF_KERNEL_TO_PA (MEM_END_HIGHER_HALF_MAP).val);
    const Physical paRegionEnd = PHYSICAL (pa.val + paRegionSizeBytes);
    const PageDirectory pd     = kpg_getcurrentpd();

    while (pa.val < paRegionEnd.val) {
        const KernelPhysicalMemoryStates state = kpmm_getPageStatus (pa);
        const PTR va                           = (PTR)HIGHER_HALF_KERNEL_TO_VA (pa);

        if (state == PMM_STATE_FREE) {
            if (kpg_unmap (pd, va) == false) {
                FATAL_BUG(); // Should not fail.
            }
            pa.val += CONFIG_PAGE_FRAME_SIZE_BYTES;
        } else if (state == PMM_STATE_USED || state == PMM_STATE_RESERVED) {
            const SIZE szPages = s_getPhysicalBlockPageCount (pa, paRegionEnd);
            if (!kvmm_memmap (g_kstate.context, va, NULL, szPages,
                              VMM_MEMMAP_FLAG_KERNEL_PAGE | VMM_MEMMAP_FLAG_COMMITTED, NULL)) {
                FATAL_BUG(); // Should not fail.
            }
            pa.val += PAGEFRAMES_TO_BYTES (szPages);
        } else {
            UNREACHABLE();
        }
    }

    // ---------------------------------------------------------------------------------------------
    // There are certain virutal addresses that are reserved for Kernel use. These are reserved
    // here.
    if (!kvmm_memmap (g_kstate.context, MEM_START_PAGING_EXT_TEMP_MAP, NULL, 1,
                      VMM_MEMMAP_FLAG_KERNEL_PAGE | VMM_MEMMAP_FLAG_COMMITTED, NULL)) {
        FATAL_BUG(); // Should not fail.
    }

    if (!kvmm_memmap (g_kstate.context, MEM_START_PAGING_INT_TEMP_MAP, NULL, 1,
                      VMM_MEMMAP_FLAG_KERNEL_PAGE | VMM_MEMMAP_FLAG_COMMITTED, NULL)) {
        FATAL_BUG(); // Should not fail.
    }
}
