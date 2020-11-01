/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel Routines
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <kernel.h>
#include <stdarg.h>

void kpanic(const char *s,...)
{
    va_list l;
    va_start(l,s);

    // Set FG=WHITE, BG=RED
    kdisp_ioctl(DISP_SETATTR, disp_attr(RED,WHITE,0));

    printk(PK_ONSCREEN,"\r\nKERNEL PANIC!!\n\r");
    vprintk(s,l);
    va_end(l);

    khalt();
}
