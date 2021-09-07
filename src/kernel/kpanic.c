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
void kpanic_gs(const char *s,...)
{
    char buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start(l,s);
    vsnprintk(buffer, ARRAY_LENGTH(buffer), s, l);
    va_end(l);

    printk_debug("%s",buffer);

    kdisp_ioctl(DISP_SETATTR, disp_attr(RED,WHITE,0));
    printk(buffer);

    khalt();
}
