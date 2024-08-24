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
#include <graphics.h>

static void display_system_info ();
static void s_initializeMemoryManagers ();
static SIZE s_getPhysicalBlockPageCount (Physical pa, Physical end);
//static void vmm_basic_testing();
//static void s_dumpPab ();
//static void paging_test_map_unmap();
//static void paging_test_temp_map_unmap();
//static void kmalloc_test();
//static void find_virtual_address();
static void process_poc();
//static void multiprocess_demo();
static void multithread_demo_kernel_thread();
static INT syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
static void graphics_demo_basic();

/* Kernel state global variable */
KernelStateInfo g_kstate;

__attribute__ ((noreturn))
void kernel_main ()
{
    FUNC_ENTRY();

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_BOOT_COMPLETE);
    // Initialize Text display
    kdisp_init ();

    // Initilaize Physical Memory Manger
    kpmm_init ();

    display_system_info ();

    kearly_println ("[  ]\tVirtual memory management.");

    ksalloc_init();

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

    kearly_printf ("\r[OK]");

    kearly_println ("[  ]\tKernel memory management.");
    kmalloc_init();
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
    //vmm_basic_testing();
    //k_halt();
    
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

#ifdef GRAPHICS_MODE_ENABLED
    if (!graphics_init()) {
        ERROR ("Graphics mode could not be enabled");
    }

    graphics_demo_basic();
    k_halt();
#endif
    process_poc();
    //new_process();
    //new_process_2();

    k_halt();
}

#ifdef GRAPHICS_MODE_ENABLED
static void graphics_drawstring (UINT x, UINT y, char* text, Color fg, Color bg)
{
    for (char* ch = text; *ch != '\0'; ch++) {
        graphics_drawfont (x, y, (UCHAR)*ch, fg, bg);
        x += CONFIG_GXMODE_FONT_WIDTH;
    }
}

static void graphics_demo_basic()
{
    #if CONFIG_GXMODE_BITSPERPIXEL == 8
        #define BG_COLOR             26
        #define FONT_FG_COLOR        29
        #define FONT_BG_COLOR        19
        #define IMAGE_BITS_PER_PIXEL 1

        #define WINDOW_BG_COLOR      169
        #define WINDOW_SHADOW_COLOR  23

        #define TITLE_BAR_BG_COLOR   125
        #define TITLE_BAR_FG_COLOR   15

        #define COLORMAP_SIZE        20 // 20x20 square
        #define COLORMAP_X           WINDOW_X + WINDOW_WIDTH - (COLORMAP_SIZE * 16) - 20
        #define COLORMAP_Y           WINDOW_Y + 20
    #elif CONFIG_GXMODE_BITSPERPIXEL == 32 || CONFIG_GXMODE_BITSPERPIXEL == 24
        #define BG_COLOR             0xAFAFAF
        #define FONT_FG_COLOR        0xDFDFDF
        #define FONT_BG_COLOR        0x2D2D2D
        #define IMAGE_BITS_PER_PIXEL 3

        #define WINDOW_BG_COLOR      0x53745F
        #define WINDOW_SHADOW_COLOR  0x7D7D7D

        #define TITLE_BAR_BG_COLOR   0x145371
        #define TITLE_BAR_FG_COLOR   0xFFFFFF

        #define MOS_LOGO_WIDTH       (130)
        #define MOS_LOGO_HEIGHT      (150)
        #define MOS_LOGO_Y           (WINDOW_Y + 10)
        #define MOS_LOGO_X           (WINDOW_X + WINDOW_WIDTH - MOS_LOGO_WIDTH - 10)
    #endif

    #define WINDOW_Y         40
    #define WINDOW_X         20
    #define WINDOW_WIDTH     CONFIG_GXMODE_XRESOLUTION - WINDOW_X - 20
    #define WINDOW_HEIGHT    CONFIG_GXMODE_YRESOLUTION - WINDOW_Y - 20

    #define TITLE_BAR_HEIGHT (CONFIG_GXMODE_FONT_HEIGHT + 6)
    #define TITLE_BAR_WIDTH  (WINDOW_WIDTH)
    #define TITLE_BAR_X      (WINDOW_X)
    #define TITLE_BAR_Y      (WINDOW_Y - TITLE_BAR_HEIGHT)

    #define CHARDUMP_Y       (WINDOW_Y + 20)
    #define CHARDUMP_X       (WINDOW_X + 20)

    if (!g_kstate.framebuffer) {
        FATAL_BUG();
    }

    // ------------------------------------
    // Draw Window and title bar
    // ------------------------------------
    graphics_rect (0, 0, CONFIG_GXMODE_XRESOLUTION, CONFIG_GXMODE_YRESOLUTION, BG_COLOR);
    graphics_rect (WINDOW_X - 6, WINDOW_Y + 6, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_SHADOW_COLOR);
    graphics_rect (WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BG_COLOR);
    graphics_rect (TITLE_BAR_X, TITLE_BAR_Y, TITLE_BAR_WIDTH, TITLE_BAR_HEIGHT, TITLE_BAR_BG_COLOR);

    char* wintitle = "MeghaOS V2 : Graphics & Fonts Demo";
    graphics_drawstring (TITLE_BAR_X + 10, TITLE_BAR_Y + 3, wintitle, TITLE_BAR_FG_COLOR,
                         TITLE_BAR_BG_COLOR);

    // ------------------------------------
    // Draw Logo image
    // ------------------------------------
    #if CONFIG_GXMODE_BITSPERPIXEL != 8
    Physical fileStart = PHYSICAL (kboot_getBootFileItem (3).startLocation);
    U8* image          = (U8*)HIGHER_HALF_KERNEL_TO_VA (fileStart);
    graphics_image_raw (MOS_LOGO_X, MOS_LOGO_Y, MOS_LOGO_WIDTH, MOS_LOGO_HEIGHT,
                        IMAGE_BITS_PER_PIXEL, image);
    #endif

    // ------------------------------------
    // Character Map dump
    // ------------------------------------
    for (UINT c = 0; c < BOOT_FONTS_GLYPH_COUNT; c++) {
        UINT y = (c / 16) * CONFIG_GXMODE_FONT_HEIGHT * 2;
        UINT x = (c % 16) * CONFIG_GXMODE_FONT_WIDTH * 2;
        graphics_drawfont (x + CHARDUMP_X, y + CHARDUMP_Y, (UCHAR)c, FONT_FG_COLOR,
                           WINDOW_BG_COLOR);
    }

    // ------------------------------------
    // Color Map dump
    // ------------------------------------
    #if CONFIG_GXMODE_BITSPERPIXEL == 8
    for (UINT c = 0; c < 256; c++) {
        UINT y = (c / 16) * 20;
        UINT x = (c % 16) * 20;
        graphics_rect (x + COLORMAP_X, y + COLORMAP_Y, COLORMAP_SIZE, COLORMAP_SIZE, c);
    }
    #endif

    // ------------------------------------
    // Printing string
    // ------------------------------------
    char* text = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.";
    UINT y     = WINDOW_Y + WINDOW_HEIGHT - CONFIG_GXMODE_FONT_HEIGHT - 10;
    graphics_drawstring (WINDOW_X + 20, y, text, FONT_FG_COLOR, WINDOW_BG_COLOR);

    text = "the quick brown fox jumps over the lazy dog.";
    y -= CONFIG_GXMODE_FONT_HEIGHT + 5;
    graphics_drawstring (WINDOW_X + 20, y, text, FONT_FG_COLOR, WINDOW_BG_COLOR);
}
#endif // GRAPHICS_MODE_ENABLED

//static void vmm_basic_testing()
//{
//    FUNC_ENTRY();
//
//    INFO ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
//    INFO ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
//    INFO ("Used salloc bytes: %x bytes", ksalloc_getUsedMemory());
//
//    // Physical kernelPD = HIGHER_HALF_KERNEL_TO_PA(MEM_START_KERNEL_PAGE_DIR);
//    // VMemoryManager* vmm = kvmm_new (0xC03E8000, 0xC03EA000, kernelPD, PMM_REGION_ANY);
//    VMemoryManager* vmm = g_kstate.context;
//
//    Physical newPA;
//    kpmm_alloc(&newPA, 2, PMM_REGION_ANY);
//
//    int* addr = (int*)kvmm_memmap (vmm, (PTR)NULL, &newPA, 2,
//                                   VMM_MEMMAP_FLAG_IMMCOMMIT | VMM_MEMMAP_FLAG_KERNEL_PAGE, &newPA);
//
//    INFO ("Allocated Physical page: %px", newPA.val);
//
//    kvmm_printVASList (vmm);
//
//    *addr = 10;
//    INFO ("Reading from %px. Value is: %u", addr, *addr);
//
//    // Now delete allocated vm address space
//    kvmm_free(vmm, (PTR)addr);
//
//    //
//    //    int* addr = (int*)kvmm_alloc (vmm, 1, PG_MAP_FLAG_KERNEL_DEFAULT,
//    //    VMM_ADDR_SPACE_FLAG_NONE); INFO ("Allocated address: %px", addr);
//    //
//    //    // Commit page here only
//    //    PageDirectory pd = kpg_getcurrentpd();
//    //    Physical pa;
//    //
//    //    kpmm_alloc (&pa, 1, PMM_REGION_ANY);
//    //
//    //    PTR va        = 0xC03E8000;
//    //    PTR pageStart = ALIGN_DOWN (va, CONFIG_PAGE_FRAME_SIZE_BYTES);
//    //    kpg_map (pd, pageStart, pa, PG_MAP_FLAG_KERNEL_DEFAULT);
//    //    INFO ("Commit successful for VA: %px", va);
//    //
//    //    // Test allocation
//    //    kvmm_printVASList (vmm);
//    //
//    //    *addr = 10;
//    //    kdebug_println ("Value is %u", *addr);
//    //
//    //    // Now delete allocated vm address space
//    //    // kvmm_free(vmm, va);
//    //
//    //    //// Test Deallocation
//    //    // kvmm_printVASList (vmm);
//    //
//    //    // kdebug_println ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
//    //    // kdebug_println ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
//    //    // kdebug_println ("Used salloc bytes: %x bytes", salloc_getUsedMemory());
//    //
//    //    // Now delete complete VMM
//    //    kvmm_delete (&vmm);
//    //
//    INFO ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
//    INFO ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
//    INFO ("Used salloc bytes: %x bytes", ksalloc_getUsedMemory());
//}

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
//    void* startAddress_va = HIGHER_HALF_KERNEL_TO_VA (startAddress);
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

    BootFileItem fileinfo = kboot_getBootFileItem (2);
    Physical startAddress = PHYSICAL (fileinfo.startLocation);
    SIZE lengthBytes      = (SIZE)fileinfo.length;

    kearly_println ("\n------ [ Cooperative Multithreading Demo ] ------\n");

    INFO ("Process: Phy start: %px, Len: %x bytes", startAddress.val, lengthBytes);
    kdebug_println ("Free RAM bytes: %x bytes", kpmm_getFreeMemorySize());
    kdebug_println ("Used Kmalloc bytes: %x bytes", kmalloc_getUsedMemory());
    kdebug_println ("Used salloc bytes: %x bytes", ksalloc_getUsedMemory());

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
    kdebug_println ("Used salloc bytes: %x bytes", ksalloc_getUsedMemory());

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
    UNREACHABLE();
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
        k_panicOnError(); // Physical memory allocation must pass.
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
                k_panicOnError(); // Unmap must not fail.
            }
            pa.val += CONFIG_PAGE_FRAME_SIZE_BYTES;
        } else if (state == PMM_STATE_USED || state == PMM_STATE_RESERVED) {
            const SIZE szPages = s_getPhysicalBlockPageCount (pa, paRegionEnd);
            if (!kvmm_memmap (g_kstate.context, va, NULL, szPages,
                              VMM_MEMMAP_FLAG_KERNEL_PAGE | VMM_MEMMAP_FLAG_COMMITTED, NULL)) {
                FATAL_BUG(); // Should not fail.
            }
            pa.val += PAGEFRAMES_TO_BYTES (szPages);
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
