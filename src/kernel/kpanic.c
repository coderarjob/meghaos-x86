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

    kdisp_ioctl(DISP_SETATTR, disp_attr(RED,WHITE,0));
    printk(PK_ONSCREEN,"\r\nKernel Panic!\n\r");
    vprintk(PK_ONSCREEN, s,l);

    printk(PK_DEBUG,"\r\nKernel Panic!\n\r");
    vprintk(PK_DEBUG, s,l);
    va_end(l);

    khalt();
}
