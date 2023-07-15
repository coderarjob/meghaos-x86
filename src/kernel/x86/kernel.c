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

#include <disp.h>
#include <stdarg.h>
#include <limits.h>
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


static void usermode_main ();
static void display_system_info ();
static void s_markUsedMemory ();
static void s_dumpPab ();

/*
 Virtual memory        ->  Physical memory
 0xc0000000 - 0xc03fffff -> 0x000000000000 - 0x0000003fffff
*/
//volatile CHAR *a = (CHAR *)0x00300000;
volatile CHAR *a = (CHAR *)0xC0300000;

__attribute__ ((noreturn)) 
void kernel_main ()
{
    kdisp_init ();
    kearly_printf ("\r\n[OK]\tPaging enabled.");

    // Initilaize Physical Memory Manger
    kpmm_init ();

    // Mark memory already occupied by the modules.
    s_markUsedMemory();

    // TSS setup
    kearly_printf ("\r\n[  ]\tTSS setup.");
    ktss_init ();
    kearly_printf ("\r[OK]");

    // Usermode code segment
    kearly_printf ("\r\n[  ]\tUser mode GDT setup.");
    kgdt_edit (GDT_INDEX_UCODE, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit (GDT_INDEX_UDATA, 0, 0xFFFFF, 0xF2, 0xD);
    kgdt_write ();
    kearly_printf ("\r[OK]");

    // Setup IDT
    kearly_printf ("\r\n[  ]\tIDT setup");
    kidt_init ();

    kidt_edit (0, div_zero_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
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

    // Jump to user mode
    kearly_printf ("\r\nJumping to User mode..");
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,CYAN,0));

    jump_to_usermode (GDT_SELECTOR_UDATA, GDT_SELECTOR_UCODE, &usermode_main);
    while (1);
    
}

void s_dumpPab ()
{
#if DEBUG
    U8 *s_pab = (U8 *)CAST_PA_TO_VA (g_pab);
    UINT bytes = 120;

    while (bytes)
    {
        kdebug_printf ("\r\n%x:", s_pab);
        for (int i = 0; i < 16 && bytes; bytes--, i+=2, s_pab+=2)
            kdebug_printf ("\t%x:%x ", *(s_pab + 1), *s_pab);
    }
#endif // DEBUG
}

void display_system_info ()
{
#if DEBUG
    BootLoaderInfo *mi = kboot_getCurrentBootLoaderInfo ();
    INT loadedFilesCount = kBootLoaderInfo_getFilesCount (mi);

    kdebug_printf ("%s","\r\nLoaded kernel files:");
    for (INT i = 0; i < loadedFilesCount; i++){
        BootFileItem* file = kBootLoaderInfo_getFileItem (mi, i);
        UINT startLocation = (UINT)kBootFileItem_getStartLocation (file);
        UINT length_bytes = (UINT)kBootFileItem_getLength (file);

        kdebug_printf ("\r\n* file: Start = %x, Length = %x",
                startLocation, length_bytes);
    }


    INT memoryMapItemCount = kBootLoaderInfo_getMemoryMapItemCount (mi);
    kdebug_printf ("%s","\r\nBIOS Memory map:"); 
    U64 installed_memory = 0;
    for (INT i = 0; i < memoryMapItemCount; i++)
    {
        BootMemoryMapItem* item = kBootLoaderInfo_getMemoryMapItem (mi, i);
        U64 baseAddress = kBootMemoryMapItem_getBaseAddress (item);
        U64 length_bytes = kBootMemoryMapItem_getLength (item);
        BootMemoryMapTypes type = kBootMemoryMapItem_getType (item);

        installed_memory += length_bytes;
        kdebug_printf ("\r\n* map: Start = %llx, Length = %llx, Type = %u",
                         baseAddress, length_bytes, type);
    }

    kdebug_printf ("\r\nKernel files loaded: %u", loadedFilesCount);
    kdebug_printf ("\r\nMax RAM Pages: %u", MAX_PAB_ADDRESSABLE_PAGE_COUNT);
    kdebug_printf ("\r\nInstalled RAM bytes: x:%llx bytes",installed_memory);
    UINT installed_memory_pageCount = (UINT)BYTES_TO_PAGEFRAMES_CEILING (installed_memory);
    kdebug_printf ("\r\nInstalled RAM Pages: %u", installed_memory_pageCount);
    kdebug_printf ("\r\nFree RAM bytes: x:%llx bytes", kpmm_getFreeMemorySize ());
#endif // DEBUG
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
static void s_markUsedMemory ()
{
    BootLoaderInfo *bootloaderinfo = kboot_getCurrentBootLoaderInfo ();
    INT filesCount = kBootLoaderInfo_getFilesCount (bootloaderinfo);
    for (INT i = 0; i < filesCount; i++)
    {
        BootFileItem* fileinfo = kBootLoaderInfo_getFileItem (bootloaderinfo, i);
        USYSINT startAddress = (USYSINT)kBootFileItem_getStartLocation (fileinfo);
        USYSINT lengthBytes = (USYSINT)kBootFileItem_getLength (fileinfo);
        UINT pageFrameCount = BYTES_TO_PAGEFRAMES_CEILING (lengthBytes);

        kdebug_printf ("\r\nI: Allocate startAddress: %px, byteCount: %px, pageFrames: %u."
                        , startAddress, lengthBytes, pageFrameCount);
        if (kpmm_allocAt (createPhysical(startAddress), pageFrameCount, PMM_REGION_ANY) == false)
            k_assertOnError ();
    }
}

void usermode_main ()
{
    kearly_printf ("\r\nInside usermode..");

    kbochs_breakpoint();
    //__asm__ volatile ("CALL 0x1B:%0"::"p"(sys_dummy_asm_handler));
    __asm__ volatile ("INT 0x40");
    kbochs_breakpoint();

    kearly_printf ("\r\nLocation of kernel_main = %x", kernel_main);
    kearly_printf ("\r\nLocation of pab = %x",CAST_PA_TO_VA (g_pab));

    extern void display_PageInfo ();
    display_PageInfo();

    *a = 0;

    while (1);
}

