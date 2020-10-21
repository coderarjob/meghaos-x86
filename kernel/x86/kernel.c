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

__attribute__((noreturn)) 
void __main()
{
    kprintf("Hex: %x", 0xabcd);

    //kprintf("Note: %s Hex: 0x%x","Kernel Loaded",0xDABC);

    //kputs("Kernel ready..",VGA_TEXT_GREY);
    //kprinthex(0xFFAB);
    while(1);
}
