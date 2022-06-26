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

#include <kernel.h>
#include <stdarg.h>
#include <limits.h>

static void usermode_main ();
void __jump_to_usermode (U32 dataselector, 
                        U32 codeselector, void (*user_func)());
static void div_zero ();
static void sys_dummy ();
static void fault_gp ();
static void page_fault ();
static void display_system_info ();

/*
 Virtual memory        ->  Physical memory
 0xc0000000 - 0xc03fffff -> 0x000000000000 - 0x0000003fffff
*/
//volatile CHAR *a = (CHAR *)0x00300000;
volatile CHAR *a = (CHAR *)0xc0400000;

__attribute__ ((noreturn)) 
void __kernel_main ()
{
    kpmm_init ();

    kdisp_init ();
    kearly_printf ("\r\n[OK]\tPaging enabled.");

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

    kidt_edit (0, div_zero, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (14, page_fault, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (13, fault_gp,GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 0);
    kidt_edit (0x40, sys_dummy, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE, 3);

    kearly_printf ("\r[OK]");

    // Display available memory
    display_system_info ();
    
    // Paging information
    extern void paging_print ();
    paging_print ();

    // Jump to user mode
    kearly_printf ("\r\nJumping to User mode..");
    kdisp_ioctl (DISP_SETATTR,k_dispAttr (BLACK,CYAN,0));
    __jump_to_usermode (GDT_SELECTOR_UDATA, GDT_SELECTOR_UCODE, &usermode_main);
    while (1);
    
}

void display_system_info ()
{
    BootLoaderInfo *mi = kboot_getCurrentBootLoaderInfo ();
    UINT loadedFilesCount = (UINT)kboot_getBootLoaderInfoFilesCount (mi);

    kdebug_printf ("%s","\r\nLoaded kernel files:");
    for (INT i = 0; i < loadedFilesCount; i++){
        BootFileItem* file = kboot_getBootLoaderInfoBootFileItem (mi, i);
        UINT startLocation = (UINT)kboot_getBootFileItemStartLocation (file);
        UINT length_bytes = (UINT)kboot_getBootFileItemFileLength (file);

        kdebug_printf ("\r\n* file: Start = %x, Length = %x",
                startLocation, length_bytes);
    }


    UINT memoryMapItemCount = kboot_getBootLoaderInfoBootMemoryMapItemCount (mi);
    kdebug_printf ("%s","\r\nBIOS Memory map:"); 
    U64 available_memory = 0;
    for (INT i = 0; i < memoryMapItemCount; i++)
    {
        BootMemoryMapItem* item = kboot_getBootLoaderInfoBootMemoryMapItem (mi, i);
        U64 baseAddress = kboot_getBootMemoryMapItemBaseAddress (item);
        U64 length_bytes = kboot_getBootMemoryMapItemLengthBytes (item);
        PMMAllocationTypes type = kboot_getBootMemoryMapItemType (item);

        available_memory += length_bytes;
        kdebug_printf ("\r\n* map: Start = %llx, Length = %llx, Type = %u",
                         baseAddress, length_bytes, type);
    }

    kdebug_printf ("\r\nKernel files loaded: %u", loadedFilesCount);
    kdebug_printf ("\r\nAvailable RAM bytes: %u KiB",available_memory/1024);

    UINT availablePageCount = BYTES_TO_PAGEFRAMES (available_memory);
    kdebug_printf ("\r\nAvailable RAM Pages: %u", availablePageCount);

    kdebug_printf ("\r\nMax RAM Pages: %u", MAX_ADDRESSABLE_PAGE);
}

__attribute__ ((noreturn))
void page_fault ()
{
    register INT fault_addr;
    INT errorcode;
    /* GCC does not preserve ESP in EBP for function with no arguments.
     * This is the reason I am doing ESP + 0x24 (listing shows GCC does ESP - 0x24
     * as the first instruction in this function)
     * TODO: Implement the page_fault handler in assembly and then call a C
     * function for printing messages etc.
     * */
    __asm__ volatile ("mov %%eax, [%%esp + 0x24]\r\n"
                      "mov %0, %%eax":"=m"(errorcode)::"eax");
    __asm__ volatile ("mov %0, %%cr2":"=r"(fault_addr));

    k_panic ("Page fault when accessing address 0x%x (error: 0x%x)",
            fault_addr,errorcode);
}

void sys_dummy ()
{
    kearly_printf ("\r\nInside sys_dummy routine..");
    outb (0x80,4);
    // Needs to IRET not RET
}

__attribute__ ((noreturn))
void fault_gp ()
{
    k_panic ("%s","General protection fault."); 
}

__attribute__ ((noreturn))
void div_zero ()
{
    k_panic ("%s","Error: Division by zero"); 
}

void usermode_main ()
{
    //__asm__ volatile ("INT 0x40");

    kearly_printf ("\r\nInside usermode..");

    kearly_printf ("\r\nUINT_MAX %llu\r\nULONG_MAX %llu\r\nULLONG_MAX %llu",
            (U64)UINT_MAX,
            (U64)ULONG_MAX,
            (U64)ULLONG_MAX);

    kearly_printf ("\r\n%u,%x,%o,%s,%%",
                        45789,
                        0xcafeefe,
                        02760,
                        "Hello Arjob");
    U64 var = 0xCF010203040506FF;   
    kearly_printf ("\r\n%llx",var);
    kearly_printf ("\r\nLocation of __kernel_main = %x",__kernel_main);

    //k_assert (1 < 0,"Nonsense");
    *a = 0;    

    while (1);
}

