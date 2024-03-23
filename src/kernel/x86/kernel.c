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
#include <x86/kernel.h>
#include <panic.h>
#include <paging.h>
#include <utils.h>
#include <x86/memloc.h>
#include <memmanage.h>

static void usermode_main ();
static void display_system_info ();
static void s_markUsedMemory ();
static void s_dumpPab ();
//static void paging_test_map_unmap();
//static void paging_test_temp_map_unmap();
//static void kmalloc_test();
static void s_unmapInitialUnusedAddressSpace(Physical start, Physical end);

__attribute__ ((noreturn)) 
void kernel_main ()
{
    kdisp_init ();
    kearly_println ("[OK]\tPaging enabled.");

    // Initilaize Physical Memory Manger
    kpmm_init ();

    // Mark memory already occupied by the modules and unmap unused Virutal pages.
    s_markUsedMemory();

    salloc_init();
    kmalloc_init();

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

    // Jump to user mode
    INFO ("Jumping to User mode..");
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,CYAN,0));

    jump_to_usermode (GDT_SELECTOR_UDATA, GDT_SELECTOR_UCODE, &usermode_main);
    while (1);
}

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

void setup_paging()
{
    /* Create new PD, PT at where? */
    //paging_recurseSetup (PD)

    /* NOTE: PD must have physically backed PT for the addresses passed in to paging_map. Later on
     * with demand creation, unbacked PTs will work.*/
    //paging_setPT (PD, 3GB, PT);

    /* Higher half map
     * - NULL            >> (0          ) to (4KB        )
     * - 0 to 268KB      >> (3GB        ) to (3GB + 268KB)    Physically backed
     * - 640KB to 1MB    >> (3GB + 640KB) to (3GB + 1MB)      Physically backed (reserved section)
     * - 1MB to KBIN_END >> (3GB + 1MB  ) to (3GB + KBIN_END) Physically backed*/
    //paging_map (PD, NULL, 0, 1, UNUSED);
    //paging_map (PD, BACKED, 3GB, 268KB, &PHYSICAL(0));
    //paging_map (PD, BACKED, 3GB+640KB, (1MB-640KB), &PHYSICAL(640KB));
    //paging_map (PD, BACKED, 3GB+1MB, KBIN_END, &PHYSICAL(1MB));
}

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
    FUNC_ENTRY("start: 0x%px, end: 0x%px", start.val, end.val);

    PageDirectory pd      = kpg_getcurrentpd();
    PTR           startva = (PTR)CAST_PA_TO_VA (start);
    PTR           endva   = (PTR)CAST_PA_TO_VA (end);

    k_assert (IS_ALIGNED (startva, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address not page aligned");
    k_assert (IS_ALIGNED (endva, CONFIG_PAGE_FRAME_SIZE_BYTES), "Address not page aligned");

    for (PTR va = startva; va < endva; va += CONFIG_PAGE_FRAME_SIZE_BYTES)
    {
        kpg_unmap (pd, va);
        k_assertOnError();
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

    /* Kernel reserved */
    // TODO: Should be a paging function to get physcical memory from virtual memory.
    Physical kernel_low_region_end_phy = PHYSICAL(KERNEL_LOW_REGION_END - 0xC0000000);
    UINT pageCount = BYTES_TO_PAGEFRAMES_CEILING (kernel_low_region_end_phy.val);
    if (kpmm_allocAt (createPhysical (0), pageCount, PMM_REGION_ANY) == false)
        k_panic ("Kernel low region of physical memory must be free.");

    /* Remove unnecessory vritual page mappings (400KiB to 640 KiB)*/
    s_unmapInitialUnusedAddressSpace (kernel_low_region_end_phy,
                                      createPhysical (640 * KB));

    /* Module files */
    USYSINT totalModuleSizeBytes   = 0;
    BootLoaderInfo* bootloaderinfo = kboot_getCurrentBootLoaderInfo();
    INT filesCount                 = kBootLoaderInfo_getFilesCount (bootloaderinfo);
    for (INT i = 0; i < filesCount; i++)
    {
        BootFileItem* fileinfo = kBootLoaderInfo_getFileItem (bootloaderinfo, i);
        USYSINT startAddress   = (USYSINT)kBootFileItem_getStartLocation (fileinfo);
        USYSINT lengthBytes    = (USYSINT)kBootFileItem_getLength (fileinfo);
        UINT pageFrameCount    = BYTES_TO_PAGEFRAMES_CEILING (lengthBytes);
        totalModuleSizeBytes += lengthBytes;

        if (kpmm_allocAt (createPhysical (startAddress), pageFrameCount, PMM_REGION_ANY) == false)
            k_assertOnError();
    }

    /* Remove unnecessory vritual page mappings (ModuleFilesEnd to 2 MiB)*/
    s_unmapInitialUnusedAddressSpace (
        createPhysical ((1 * MB) + ALIGN_UP (totalModuleSizeBytes, CONFIG_PAGE_FRAME_SIZE_BYTES)),
        createPhysical (2 * MB));
}

void usermode_main ()
{
    FUNC_ENTRY();

    INFO ("Inside usermode..");

    //__asm__ volatile ("CALL 0x1B:%0"::"p"(sys_dummy_asm_handler));
    __asm__ volatile ("INT 0x40");

    INFO ("Location of kernel_main = %x", kernel_main);
    INFO ("Location of pab = %x", CAST_PA_TO_VA (g_pab));
    INFO ("Size of PTR = %u", sizeof (PTR));
    INFO ("MASK(19,0) is %x", BIT_MASK (19, 0));

    /*kbochs_breakpoint();
    Physical pa = PHYSICAL(0x100000);
    PageDirectory pd = kpg_getcurrentpd();
    kpg_map(&pd, PAGE_MAP_BACKED, 0xC01FF000, 1, &pa);*/
    kearly_println("%x", (PTR)salloc(10));
    kearly_println("%x", (PTR)salloc(4096));
    kearly_println("%x", (PTR)salloc(2));
    kearly_println("%x", (PTR)salloc(1));
    INFO ("PD: %x, PT: %x", g_page_dir.val, g_page_table.val);
    kearly_println ("Satic memory allocations: %u bytes", salloc_getUsedMemory());
    kearly_println ("Kernel heap memory allocations: %u bytes", kmalloc_getUsedMemory());
//    volatile CHAR* a = (CHAR*)0xC0400000;
//    *a = 0;

    k_halt();
}

