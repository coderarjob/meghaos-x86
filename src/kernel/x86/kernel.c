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

static void display_system_info ();
static void s_initializeMemoryManagers ();
//static void s_dumpPab ();
//static void paging_test_map_unmap();
//static void paging_test_temp_map_unmap();
//static void kmalloc_test();
//static void find_virtual_address();
static void process_poc();
//static void multiprocess_demo();
static void multithread_demo_kernel_thread();
static INT syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);

/* Kernel state global variable */
KernelStateInfo g_kstate;

__attribute__ ((noreturn))
void kernel_main ()
{
    FUNC_ENTRY();

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_BOOT_COMPLETE);
    g_kstate.kernelPageDirectory = HIGHER_HALF_KERNEL_TO_PA(MEM_START_KERNEL_PAGE_DIR);

    // Initialize Text display
    kdisp_init ();

    // Initilaize Physical Memory Manger
    kpmm_init ();

    display_system_info ();

    kearly_println ("[  ]\tMemory management.");

    salloc_init();

    // Initialize VMM
    g_kstate.kernelVMM = vmm_new (MEM_START_KERNEL_LOW_REGION, MEM_END_KERNEL_HIGH_REGION);

    // Mark memory already occupied by the modules and unmap unused Virutal pages.
    s_initializeMemoryManagers();

    kmalloc_init();
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

    kearly_printf ("\r[OK]");

    kprocess_init();

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_KERNEL_READY);

    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,GREEN,0));
    kearly_println ("Kernel initialization finished..");
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,LIGHT_GRAY,0));

    // Display available memory
    //s_dumpPab();
    // Paging information
    //extern void paging_print ();
    //paging_print ();
    //paging_test_map_unmap();
    //paging_test_temp_map_unmap();
    //kmalloc_test();

    //find_virtual_address();
    //display_system_info ();
    //s_dumpPab();
    
    process_poc();
    //new_process();
    //new_process_2();

    k_halt();
}

//static void multiprocess_demo()
//{
//    FUNC_ENTRY();
//
//    BootLoaderInfo* bootloaderinfo = kboot_getCurrentBootLoaderInfo();
//    BootFileItem* fileinfo         = kBootLoaderInfo_getFileItem (bootloaderinfo, 1);
//    Physical startAddress          = PHYSICAL (kBootFileItem_getStartLocation (fileinfo));
//    SIZE lengthBytes               = (SIZE)kBootFileItem_getLength (fileinfo);
//
//    kearly_println ("\n------ [ Cooperative Multithreading Demo ] ------\n");
//
//    INFO ("Process: Phy start: %px, Len: %x bytes", startAddress.val, lengthBytes);
//    kdebug_println ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
//    kdebug_println ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
//    kdebug_println ("Used salloc bytes: %x bytes", salloc_getUsedMemory());
//
//    void* startAddress_va = CAST_PA_TO_VA (startAddress);
//    INT processID = syscall (1, (PTR)startAddress_va, lengthBytes, PROCESS_FLAGS_NONE, 0, 0);
//    if (processID < 0) {
//        k_panicOnError();
//    }
//
//    INFO ("Process ID: %u", processID);
//
//    for (int i = 0; i < 8; i++)
//    {
//        kearly_println("Kernel thread - Yielding");
//        syscall (2, 0, 0, 0, 0, 0);
//    }
//
//    kearly_println ("Kernel thread - Exiting.");
//    syscall (3, 0, 0, 0, 0, 0);
//
//    kearly_println ("Kernel thread - Not exited. It is the only process.");
//
//    kdebug_println ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
//    kdebug_println ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
//    kdebug_println ("Used salloc bytes: %x bytes", salloc_getUsedMemory());
//
//    kearly_println ("------ [ END ] ------");
//
//    k_halt();
//}

static void multithread_demo_kernel_thread()
{
    FUNC_ENTRY();

    BootLoaderInfo* bootloaderinfo = kboot_getCurrentBootLoaderInfo();
    BootFileItem* fileinfo         = kBootLoaderInfo_getFileItem (bootloaderinfo, 2);
    Physical startAddress          = PHYSICAL (kBootFileItem_getStartLocation (fileinfo));
    SIZE lengthBytes               = (SIZE)kBootFileItem_getLength (fileinfo);

    kearly_println ("\n------ [ Cooperative Multithreading Demo ] ------\n");

    INFO ("Process: Phy start: %px, Len: %x bytes", startAddress.val, lengthBytes);
    kdebug_println ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
    kdebug_println ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
    kdebug_println ("Used salloc bytes: %x bytes", salloc_getUsedMemory());

    void* startAddress_va = HIGHER_HALF_KERNEL_TO_VA (startAddress);
    INT processID = syscall (1, (PTR)startAddress_va, lengthBytes, PROCESS_FLAGS_NONE, 0, 0);
    if (processID < 0) {
        k_panicOnError();
    }

    INFO ("Process ID: %u", processID);

    // ----------------------
    UINT max = 12 * MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        syscall (2, 0, 0, 0, 0, 0);
    }
    // ----------------------

    kdisp_ioctl (DISP_SETATTR, k_dispAttr (BLACK, LIGHT_GRAY, 0));
    kdisp_ioctl (DISP_SETCOORDS, 40, 0);

    kearly_println ("Kernel thread - Exiting.");
    syscall (3, 0, 0, 0, 0, 0);

    kearly_println ("Kernel thread - Not exited. It is the only process.");

    kdebug_println ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
    kdebug_println ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
    kdebug_println ("Used salloc bytes: %x bytes", salloc_getUsedMemory());

    kearly_println ("------ [ END ] ------");

    k_halt();
}

static void process_poc()
{
    FUNC_ENTRY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    void* startAddress_va = multithread_demo_kernel_thread;
    //void* startAddress_va = multiprocess_demo;
#pragma GCC diagnostic pop

    INT processID = kprocess_create (startAddress_va, 0,
                                     PROCESS_FLAGS_THREAD | PROCESS_FLAGS_KERNEL_PROCESS);
    if (processID < 0) {
        k_panicOnError();
    }

    INFO ("Process ID: %u", processID);

    kprocess_yield (NULL);
}

static INT syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5)
{
    INT retval = 0;
    __asm__ volatile("int 0x50"
                     : "=a"(retval) // This is required. Otherwise compiler will not know that eax
                                    // will be changed after this instruction.
                     : "a"(fn), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
                     :);
    return retval;
}

//static void find_virtual_address()
//{
//    FUNC_ENTRY();
//
//    SIZE numPages = 3000;
//    // void* addr = kpg_findVirtualAddressSpace(kpg_getcurrentpd(), 2000, 0xC03FF000, 0xC0900000);
//    PTR addr = kpg_findVirtualAddressSpace (kpg_getcurrentpd(), numPages, 0xC00A0000, 0xC0F00000);
//    kearly_println ("Found address is %px", addr);
//
//    for (UINT i = 0; i < numPages; addr += CONFIG_PAGE_FRAME_SIZE_BYTES, i++)
//    {
//        Physical pa;
//        if (!kpmm_alloc(&pa, 1, PMM_REGION_ANY)) {
//            k_panicOnError();
//        }
//
//        if (!kpg_map(kpg_getcurrentpd(), addr, pa, PG_MAP_FLAG_KERNEL|PG_MAP_FLAG_WRITABLE)) {
//            k_panicOnError();
//        }
//    }
//
//    addr = (PTR)kpg_findVirtualAddressSpace (kpg_getcurrentpd(), 200, 0xC00A0000, 0xC0900000);
//    kearly_println ("Found address is %px", addr);
//}

//static void kmalloc_test()
//{
//    int* addr1 = (int*)kmalloc(10 * sizeof(UINT));
//    int* addr2 = (int*)kmalloc(100 * sizeof(UINT));
//
//    *addr1 = 0xB001;
//    *addr2 = 0xC002;
//
//    INFO ("addr1: %px, addr2: %px", addr1, addr2);
//
//    kfree(addr1);
//    kfree(addr2);
//}
//
//static void paging_test_map_unmap()
//{
//    FUNC_ENTRY();
//
//    volatile CHAR* a = (CHAR*)0xC0400000;
//
//    Physical pa;
//    kpmm_alloc (&pa, 1, PMM_REGION_ANY);
//    PageDirectory pd = kpg_getcurrentpd();
//    kpg_map (pd, (PTR)a, pa, PG_MAP_FLAG_WRITABLE);
//    k_assertOnError();
//
//    *a = 0;
//
//    kpg_unmap (pd, (PTR)a);
//    k_assertOnError();
//
//    kpmm_free(pa, 1);
//    k_assertOnError();
//}
//
//static void paging_test_temp_map_unmap()
//{
//    FUNC_ENTRY();
//
//    Physical pa1, pa2;
//    kpmm_alloc (&pa1, 1, PMM_REGION_ANY);
//    kpmm_alloc (&pa2, 1, PMM_REGION_ANY);
//
//    void* addr = kpg_temporaryMap (pa1);
//    //x86_TLB_INVAL_SINGLE (0xC03FF000U);
//    //tlb_inval_complete();
//    *(INT*)addr = 0xF001;
//    kpg_temporaryUnmap();
//
//    addr = kpg_temporaryMap (pa2);
//    //x86_TLB_INVAL_SINGLE (0xC03FF000U);
//    //tlb_inval_complete();
//    *(INT*)addr = 0xF002;
//    kpg_temporaryUnmap();
//
//    kdebug_println ("Read Value at %x", pa1.val);
//    kdebug_println ("Read Value at %x", pa2.val);
//    kbochs_breakpoint();
//
//    kpmm_free(pa1, 1);
//    k_assertOnError();
//
//    kpmm_free(pa2, 1);
//    k_assertOnError();
//}

//void s_dumpPab()
//{
//    FUNC_ENTRY();
//
//#if DEBUG
//    U8*  s_pab = (U8*)CAST_PA_TO_VA (g_pab);
//    UINT bytes = 120;
//
//    while (bytes)
//    {
//        kdebug_println ("%h:", s_pab);
//        for (int i = 0; i < 16 && bytes; bytes--, i += 2, s_pab += 2)
//            kdebug_printf ("\t%h:%h ", *s_pab, *(s_pab + 1));
//    }
//#endif // DEBUG
//}

void display_system_info()
{
    FUNC_ENTRY();

#if DEBUG_LEVEL & 0x1
    BootLoaderInfo* mi               = kboot_getCurrentBootLoaderInfo();
    INT             loadedFilesCount = kBootLoaderInfo_getFilesCount (mi);

    INFO ("Loaded kernel files:");
    for (INT i = 0; i < loadedFilesCount; i++)
    {
        BootFileItem* file          = kBootLoaderInfo_getFileItem (mi, i);
        UINT          startLocation = (UINT)kBootFileItem_getStartLocation (file);
        UINT          length_bytes  = (UINT)kBootFileItem_getLength (file);

        INFO ("* file: Start = %x, Length = %u bytes", startLocation, length_bytes);
    }

    INT memoryMapItemCount = kBootLoaderInfo_getMemoryMapItemCount (mi);
    INFO ("BIOS Memory map:");
    U64 installed_memory = 0;
    for (INT i = 0; i < memoryMapItemCount; i++)
    {
        BootMemoryMapItem* item         = kBootLoaderInfo_getMemoryMapItem (mi, i);
        U64                baseAddress  = kBootMemoryMapItem_getBaseAddress (item);
        U64                length_bytes = kBootMemoryMapItem_getLength (item);
        BootMemoryMapTypes type         = kBootMemoryMapItem_getType (item);

        installed_memory += length_bytes;
        INFO ("* map: Start = %llx, Length = %llx, Type = %u", baseAddress, length_bytes, type);
    }

    INFO ("Kernel files loaded: %u", loadedFilesCount);
    INFO ("Max RAM Pages: %u", MAX_PAB_ADDRESSABLE_PAGE_COUNT);
    INFO ("Installed RAM bytes: %llx bytes", installed_memory);
    UINT installed_memory_pageCount = (UINT)BYTES_TO_PAGEFRAMES_CEILING (installed_memory);
    INFO ("Installed RAM Pages: %u", installed_memory_pageCount);
    INFO ("Free RAM bytes: :%llx bytes", kpmm_getFreeMemorySize());
#endif
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
    BootLoaderInfo* bootloaderinfo = kboot_getCurrentBootLoaderInfo();
    INT filesCount                 = kBootLoaderInfo_getFilesCount (bootloaderinfo);

    BootFileItem* fileinfo      = kBootLoaderInfo_getFileItem (bootloaderinfo, 0);
    Physical first_startAddress = PHYSICAL (kBootFileItem_getStartLocation (fileinfo));

    fileinfo                     = kBootLoaderInfo_getFileItem (bootloaderinfo, filesCount - 1);
    Physical last_startAddress   = PHYSICAL (kBootFileItem_getStartLocation (fileinfo));
    SIZE last_lengthBytes        = (USYSINT)kBootFileItem_getLength (fileinfo);
    SIZE totalModulesLengthBytes = (last_startAddress.val - first_startAddress.val) +
                                   last_lengthBytes;

    UINT pageFrameCount = BYTES_TO_PAGEFRAMES_CEILING (totalModulesLengthBytes);

    INFO ("Total size of module files: %x bytes", totalModulesLengthBytes);

    if (kpmm_allocAt (first_startAddress, pageFrameCount, PMM_REGION_ANY) == false) {
        k_panicOnError(); // Physical memory allocation must pass.
    }

    // ---------------------------------------------------------------------------------------------
    // Now that the PMM holds the complete picture of all the physical memories used & reserved, we
    // use that to initialize VMM and paging within the part which Higher Half mapped.
    Physical paStart = PHYSICAL (0);
    SIZE paSize      = MIN (kpmm_getUsableMemorySize (PMM_REGION_ANY),
                            HIGHER_HALF_KERNEL_TO_PA (MEM_END_HIGHER_HALF_MAP).val);
    PageDirectory pd = kpg_getcurrentpd();

    for (Physical pa = paStart; pa.val < paSize; pa.val += CONFIG_PAGE_FRAME_SIZE_BYTES) {
        KernelPhysicalMemoryStates state = kpmm_getPageStatus (pa);
        PTR va                           = (PTR)HIGHER_HALF_KERNEL_TO_VA (pa);
        if (state == PMM_STATE_FREE) {
            if (kpg_unmap (pd, va) == false) {
                k_panicOnError(); // Unmap must not fail.
            }
        }
        if (state == PMM_STATE_USED || state == PMM_STATE_RESERVED) {
            if (vmm_reserveAt (g_kstate.kernelVMM, va, 1, PG_MAP_FLAG_KERNEL_DEFAULT, true) == 0) {
                k_panicOnError(); // must not fail.
            }
        }
    }

    // ---------------------------------------------------------------------------------------------
    // There are certain virutal addresses that are reserved for Kernel use. These are reserved
    // here.
    if (vmm_reserveAt (g_kstate.kernelVMM, MEM_START_PAGING_EXT_TEMP_MAP, 1,
                       PG_MAP_FLAG_KERNEL_DEFAULT, true) == 0) {
        k_panicOnError();
    }

    if (vmm_reserveAt (g_kstate.kernelVMM, MEM_START_PAGING_INT_TEMP_MAP, 1,
                       PG_MAP_FLAG_KERNEL_DEFAULT, true) == 0) {
        k_panicOnError();
    }
}
