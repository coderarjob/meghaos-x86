/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform kpanic function
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <kernel.h>
#include <stdarg.h>

/* Hook function called from kpanic macro. 
 * Displays an error message on the screen and Halts */
__attribute__((noreturn)) 
void __kpanic(const char *s,...)
{
    va_list l;
    va_start(l,s);

    kdisp_ioctl(DISP_SETATTR, DARK_GRAY);
    printk(PK_ONSCREEN,"\r\nKernel Panic!\n\r");
    vprintk(s,l);
    va_end(l);

    khalt();
}
