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

__attribute__((noreturn)) 
void __main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\nKernel starting..");
    
    kgdt_edit(0,0,0,0,0);
    while(1);
}
