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

static void display_system_info ();
static void s_markUsedMemory ();
static void s_dumpPab ();
//static void paging_test_map_unmap();
//static void paging_test_temp_map_unmap();
//static void kmalloc_test();
static void s_unmapInitialUnusedAddressSpace(Physical start, Physical end);
//static void find_virtual_address();
static void process_poc();
//static void new_thread_2();
static void new_thread_1();
static INT syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);

/* This variable is globally used to set error codes*/
KernelErrorCodes k_errorNumber;

__attribute__ ((noreturn)) 
void kernel_main ()
{
    kdisp_init ();

    // Initilaize Physical Memory Manger
    kpmm_init ();

    // Mark memory already occupied by the modules and unmap unused Virutal pages.
    s_markUsedMemory();

    salloc_init();
    kmalloc_init();

    kearly_println ("[OK]\tPaging enabled.");

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

    // Display available memory
    display_system_info ();
    s_dumpPab();
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

    // Jump to user mode
    INFO ("Jumping to User mode..");
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,CYAN,0));

    //jump_to_usermode (GDT_SELECTOR_UDATA, GDT_SELECTOR_UCODE, &usermode_main);
    while (1);
}

static void new_thread_1()
{
    FUNC_ENTRY();

    //#pragma GCC diagnostic push
    //#pragma GCC diagnostic ignored "-Wpedantic"
    //    void* startAddress_va = new_thread_2;
    //#pragma GCC diagnostic pop
    //
    //    INT processID = syscall (1, (PTR)startAddress_va, 0, PROCESS_FLAGS_THREAD, 0, 0);
    //    if (processID < 0) {
    //        k_panicOnError();
    //    }
    //
    //    INFO ("Process ID: %u", processID);
    //
    //    for (int i = 0; i < 5; i++) {
    //        kearly_println ("From Process 0");
    //        syscall (2, 0, 0, 0, 0, 0);
    //    }
    //
    //    INFO ("Here it ends");

    BootLoaderInfo* bootloaderinfo = kboot_getCurrentBootLoaderInfo();
    BootFileItem* fileinfo         = kBootLoaderInfo_getFileItem (bootloaderinfo, 1);
    Physical startAddress          = PHYSICAL (kBootFileItem_getStartLocation (fileinfo));
    SIZE lengthBytes               = (SIZE)kBootFileItem_getLength (fileinfo);

    INFO ("Process: Phy start: 0x%px, Len: 0x%x bytes", startAddress.val, lengthBytes);

    void* startAddress_va = CAST_PA_TO_VA (startAddress);
    INT processID = syscall (1, (PTR)startAddress_va, lengthBytes, PROCESS_FLAGS_NONE, 0, 0);
    if (processID < 0) {
        k_panicOnError();
    }

    INFO ("Process ID: %u", processID);

    for (int i = 0; i < 5; i++) {
        kearly_println ("From thread 0 - i = %u", i);
        syscall (2, 0, 0, 0, 0, 0);
    }

    INFO ("Here it ends");

    //kprocess_switch ((UINT)processID);
    k_halt();
}

//static void new_thread_2()
//{
//    FUNC_ENTRY();
//
//    for (;;) {
//        kearly_println ("From Process 1");
//        syscall (2, 0, 0, 0, 0, 0);
//    }
//    // U32 cr3 = 0xF00;
//    // x86_READ_REG (CR3, cr3);
//
//    // INFO ("Value of cr3 is 0x%px", cr3);
//
//    k_halt();
//}

static void process_poc()
{
    FUNC_ENTRY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    void* startAddress_va = new_thread_1;
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
//    kearly_println ("Found address is 0x%px", addr);
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
//    kearly_println ("Found address is 0x%px", addr);
//}

//static void kmalloc_test()
//{
//    int* addr1 = (int*)kmalloc(10 * sizeof(UINT));
//    int* addr2 = (int*)kmalloc(100 * sizeof(UINT));
//
//    *addr1 = 0xB001;
//    *addr2 = 0xC002;
//
//    INFO ("addr1: 0x%px, addr2: 0x%px", addr1, addr2);
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

void s_dumpPab()
{
    FUNC_ENTRY();

#if DEBUG
    U8*  s_pab = (U8*)CAST_PA_TO_VA (g_pab);
    UINT bytes = 120;

    while (bytes)
    {
        kdebug_println ("%x:", s_pab);
        for (int i = 0; i < 16 && bytes; bytes--, i += 2, s_pab += 2)
            kdebug_printf ("\t%x:%x ", *s_pab, *(s_pab + 1));
    }
#endif // DEBUG
}

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
    INFO ("Installed RAM bytes: x:%llx bytes", installed_memory);
    UINT installed_memory_pageCount = (UINT)BYTES_TO_PAGEFRAMES_CEILING (installed_memory);
    INFO ("Installed RAM Pages: %u", installed_memory_pageCount);
    INFO ("Free RAM bytes: x:%llx bytes", kpmm_getFreeMemorySize());
#endif
}

/***************************************************************************************************
 * Unmaps higher-half virtual pages corresponding to the input physcial memory range.
 *
 * @Input      Start physical address. Must by page aligned.
 * @Input      End physical address. Must by page aligned.
 * @return nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static void s_unmapInitialUnusedAddressSpace (Physical start, Physical end)
{
    FUNC_ENTRY ("start: 0x%px, end: 0x%px", start.val, end.val);

    PageDirectory pd = kpg_getcurrentpd();
    PTR startva      = (PTR)CAST_PA_TO_VA (start);
    PTR endva        = (PTR)CAST_PA_TO_VA (end);

    k_assert (IS_ALIGNED (startva, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address not page aligned");
    k_assert (IS_ALIGNED (endva, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address not page aligned");

    for (PTR va = startva; va < endva; va += CONFIG_PAGE_FRAME_SIZE_BYTES) {
        if (!kpg_unmap (pd, va)) {
            k_panicOnError(); // Unmap must not fail.
        }
    }
}

/***************************************************************************************************
 * Marks pages occupied by module files as occupied.
 *
 * It consults the bootloader structures and it marks memory occupied by module files.
 * If memory map length is not aligned, memory is marked allocated, till the next page boundary.
 * This means length is aligned to the next multiple of CONFIG_PAGE_FRAME_SIZE_BYTES.
 *
 * @return nothing
 * @error   On failure, processor is halted.
 **************************************************************************************************/
static void s_markUsedMemory()
{
    FUNC_ENTRY();

    /* Kernel reserved (0 to 400KiB) */
    Physical kernel_low_region_end_phy = { 0 };
    if (!kpg_getPhysicalMapping (kpg_getcurrentpd(), KERNEL_LOW_REGION_END,
                                 &kernel_low_region_end_phy)) {
        k_panicOnError(); // KERNEL_LOW_REGION_END must have physical mapping.
    }

    UINT pageCount = BYTES_TO_PAGEFRAMES_CEILING (kernel_low_region_end_phy.val);
    if (kpmm_allocAt (createPhysical (0), pageCount, PMM_REGION_ANY) == false) {
        k_panicOnError(); // Kernel low region of physical memory must be free.
    }

    /* Remove unnecessary virtual page mappings (400KiB to 640 KiB)*/
    s_unmapInitialUnusedAddressSpace (kernel_low_region_end_phy, createPhysical (640 * KB));

    /* Accounting of physical memory used by module files */
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

    INFO ("Total size of module files: 0x%x bytes", totalModulesLengthBytes);

    if (kpmm_allocAt (first_startAddress, pageFrameCount, PMM_REGION_ANY) == false) {
        k_panicOnError(); // Physical memory allocation must pass.
    }

    /* Remove unnecessory vritual page mappings (ModuleFilesEnd to 2 MiB)*/
    s_unmapInitialUnusedAddressSpace (createPhysical (ALIGN_UP (last_startAddress.val +
                                                                    last_lengthBytes,
                                                                CONFIG_PAGE_FRAME_SIZE_BYTES)),
                                      createPhysical (2 * MB));
}
