/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform k_panic function
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <panic.h>
#include <types.h>
#include <moslimits.h>
#include <utils.h>
#include <kdebug.h>
#include <disp.h>
#include <stdarg.h>

/* Hook function called from k_panic macro. 
 * Displays an error message on the screen and Halts */
__attribute__ ((noreturn)) 
void k_panic_ndu (const CHAR *s,...)
{
    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start (l,s);
    kearly_vsnprintf (buffer, ARRAY_LENGTH (buffer), s, l);
    va_end (l);

    kdebug_printf ("%s",buffer);

    kdisp_ioctl (DISP_SETATTR, k_dispAttr (RED,WHITE,0));
    kearly_printf (buffer);

    k_halt ();
}
