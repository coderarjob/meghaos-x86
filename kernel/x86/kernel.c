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
void printbase(u32 num, u32 base);

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

    // Jump to user mode
    printk(PK_ONSCREEN,"OK\r\nJumping to User mode..");
    __jump_to_usermode(GDT_SELECTOR_UDATA, 
                       GDT_SELECTOR_UCODE,
                       &usermode_main);
    while(1);
}

void usermode_main()
{
    printk(PK_ONSCREEN,"\r\nInside usermode..");
    printk(PK_ONSCREEN,"\r\n8917 = %x (hex), %d (dex), %o (oct), %b (bin)",
                        8917,8917,8917,8917);
    while(1);
}

