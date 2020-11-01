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

void kpanic(const char *s,...);

//__attribute__((noreturn)) 
void __main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\nKernel starting..");

    u8 attr;
    kdisp_ioctl(DISP_GETATTR, &attr);
    printk(PK_ONSCREEN,"\n\rAttr = %x", attr);

    kpanic("Invalid error: %s:%x.",__FILE__, __LINE__);
    while(1);
}

void kpanic(const char *s,...)
{
    va_list l;
    va_start(l,s);
    kdisp_ioctl(DISP_SETCOORDS,0,0);
    kdisp_ioctl(DISP_SETATTR, RED);
    vprintk(s,l);
    va_end(l);
}
