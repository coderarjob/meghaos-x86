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
#if ARCH == x86
#include <x86/vgatext.h>
#endif // x86

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

#if ARCH == x86
    kdisp_ioctl (DISP_SETATTR, k_dispAttr (RED,WHITE,0));
#else
    #error "Require implementation"
#endif

    kearly_printf (buffer);

    k_halt ();
}

void kpanic_show_call_trace()
{
    PTR raddrs[CONFIG_MAX_CALL_TRACE_DEPTH] = {0};
    kdebug_dump_call_trace(raddrs, ARRAY_LENGTH(raddrs));

    kdebug_printf("\r\n%s", "Call stack:");
    for (int i = 0; i < CONFIG_MAX_CALL_TRACE_DEPTH && raddrs[i] > 0; i++)
        kdebug_printf("\r\n- 0x%x", raddrs[i]);
}
