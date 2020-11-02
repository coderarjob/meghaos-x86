/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel Routines
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <kernel.h>
#include <stdarg.h>

/* Hook function called from kpanic macro. 
 * Displays an error message on the screen and Halts */
void _kpanic(const char *file, int lineno, const char *s,...)
{
    va_list l;
    va_start(l,s);

    // Set FG=WHITE, BG=RED
    kdisp_ioctl(DISP_SETATTR, disp_attr(RED,WHITE,0));
    printk(PK_ONSCREEN,"\r\nKERNEL PANIC!!\n\r");
    printk(PK_ONSCREEN,"in '%s' at 0x%x line.\n\r", file, lineno);
    vprintk(s,l);
    va_end(l);

    khalt();
}
