/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Debug console
*
* Uses the port 0xE9 hack to output characters to the linux Qemu console.
* ---------------------------------------------------------------------------
*
* Dated: 8th June 2021
*/
#include <stdarg.h>
#include <types.h>
#include <x86/io.h>
#include <moslimits.h>
#include <utils.h>
#include <disp.h>
#include <x86/vgatext.h>

#if  defined (DEBUG)

#if (DEBUG_LEVEL & 1)
static void s_qemu_debugPutString (const CHAR *string)
{
    CHAR c;
    while ((c = *string++))
        outb (0xE9, c);
}
#endif

void kdebug_printf_ndu (const CHAR *fmt, ...)
{
    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start (l,fmt);
    kearly_vsnprintf (buffer, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

#if (DEBUG_LEVEL & 1)
    // Print to E9 port 
    s_qemu_debugPutString (buffer);
#endif

#if (DEBUG_LEVEL & 2)
    // Prints to the vga buffer
    INT attr;
    kdisp_ioctl (DISP_GETATTR, &attr);

    kdisp_ioctl (DISP_SETATTR, k_dispAttr (BLACK,DARK_GRAY,0));
    kearly_printf (buffer);

    kdisp_ioctl (DISP_SETATTR, attr);
#endif
}

#endif // DEBUG
