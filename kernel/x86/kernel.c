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

void usermode_main();
void __jump_to_usermode(u32 dataselector, 
                        u32 codeselector, void(*user_func)());
void div_zero();
void sys_dummy();
void page_fault();
void __display_system_info();

volatile char *a = (char *)0x0;

__attribute__((noreturn)) 
void __kernel_main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\nPaging enabled... OK");

    // TSS setup
    ktss_init();
    printk(PK_ONSCREEN,"\r\nTSS setup... OK");

    // Usermode code segment
    kgdt_edit(GDT_INDEX_UCODE, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit(GDT_INDEX_UDATA, 0, 0xFFFFF, 0xF2, 0xD);
    kgdt_write();
    printk(PK_ONSCREEN,"\r\nUser mode GDT setup... OK");

    // Setup IDT
    kidt_init();
    kidt_edit(0,div_zero,GDT_SELECTOR_KCODE,IDT_DES_TYPE_32_INTERRUPT_GATE,0);
    kidt_edit(14,page_fault,GDT_SELECTOR_KCODE, 
              IDT_DES_TYPE_32_INTERRUPT_GATE,0);
    kidt_edit(0x40,sys_dummy,GDT_SELECTOR_KCODE,
              IDT_DES_TYPE_32_INTERRUPT_GATE,3);

    printk(PK_ONSCREEN,"\r\nIDT setup... OK");

    // Display available memory
    __display_system_info();

    // Jump to user mode
    printk(PK_ONSCREEN,"\r\nJumping to User mode..");
    __jump_to_usermode(GDT_SELECTOR_UDATA, 
                       GDT_SELECTOR_UCODE,
                       &usermode_main);
    while(1);
}

void __display_system_info()
{
    struct boot_info *mi = (struct boot_info*)BOOT_INFO_LOCATION;
    u64 available_memory = 0;

    for(int i = mi->count - 1; i > 0; i--){
        if (mi->items[i].type == 1) 
            available_memory += mi->items[i].length;
    }

    printk(PK_ONSCREEN,"\r\n    Total Memory: %dKB",available_memory/1024);
}

__attribute__((noreturn))
void page_fault()
{
    register int fault_addr;
    int errorcode;
    /* GCC does not preserve ESP in EBP for function with no arguments.
     * This is the reason I am doing ESP+0x24 (listing shows GCC does ESP-0x24
     * as the first instruction in this function)
     * TODO: Implement the page_fault handler in assembly and then call a C
     * function for printing messages etc.
     * */
    __asm__ volatile ("mov %%eax, [%%esp+0x24]\r\n"
                      "mov %0, %%eax":"=m"(errorcode)::"eax");
    __asm__ volatile ("mov %0, %%cr2":"=r"(fault_addr));
    kpanic("Page fault when accessing address 0x%x (error: 0x%x)",
            fault_addr,errorcode);
}

void sys_dummy()
{
    outb(0x80,4);
}

__attribute__((noreturn))
void div_zero()
{
    kpanic("%s","Error: Division by zero"); 
}

void usermode_main()
{
    printk(PK_ONSCREEN,"\r\nInside usermode..");
    printk(PK_ONSCREEN,"\r\n%d,%x,%o,%s,%%",
                        45789,
                        0xcafeefe,
                        02760,
                        "Hello Arjob");

    *a = 0;    

    while(1);
}

