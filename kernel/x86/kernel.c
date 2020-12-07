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

//#define p_t char*
typedef char* p_t;

__attribute__((noreturn)) 
void __main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\nKernel starting..");
    ktss_init();

    // Usermode code segment
    kgdt_edit(GDT_INDEX_UCODE, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit(GDT_INDEX_UDATA, 0, 0xFFFFF, 0xF2, 0xD);
    kgdt_write();

    // Setup IDT
    kidt_init();
    kidt_edit(0,div_zero,GDT_SELECTOR_KCODE,IDT_DES_TYPE_32_INTERRUPT_GATE,0);
    kidt_edit(0x40,sys_dummy,GDT_SELECTOR_KCODE,
              IDT_DES_TYPE_32_INTERRUPT_GATE,3);

    // Jump to user mode
    printk(PK_ONSCREEN,"OK\r\nJumping to User mode..");
    __jump_to_usermode(GDT_SELECTOR_UDATA, 
                       GDT_SELECTOR_UCODE,
                       &usermode_main);
    while(1);
}

void sys_dummy()
{
    outb(0x80,4);
}

void div_zero()
{
    kpanic("%s","Error: Division by zero"); 
}

void usermode_main()
{
    printk(PK_ONSCREEN,"\r\nInside usermode..");
    while(1);
}

