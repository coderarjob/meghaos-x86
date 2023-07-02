/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform text display functions
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

/* Message are printed on screen and if configured also on the debug console. */
void kdisp_importantPrint(char *fmt, ...)
{
    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start(l, fmt);
    kearly_vsnprintf (buffer, ARRAY_LENGTH (buffer), fmt, l);
    va_end(l);

    kdebug_printf ("%s",buffer);

#if !defined(DEBUG) || !(DEBUG_LEVEL & 2)
    kearly_printf (buffer);
#endif

}

void kdisp_show_call_trace()
{
    PTR raddrs[CONFIG_MAX_CALL_TRACE_DEPTH] = {0};
    kdebug_dump_call_trace(raddrs, ARRAY_LENGTH(raddrs));

    kdebug_printf("\r\n%s", "Call stack:");
    for (int i = 0; i < CONFIG_MAX_CALL_TRACE_DEPTH && raddrs[i] > 0; i++)
        kdebug_printf("\r\n- 0x%x", raddrs[i]);
}
