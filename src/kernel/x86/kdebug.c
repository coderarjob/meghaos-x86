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
#include <kdebug.h>

#if (DEBUG_LEVEL & 1)
static void s_qemu_debugPutString (const CHAR *string)
{
    CHAR c;
    while ((c = *string++))
        outb (0xE9, c);
}
#endif

#if  defined (DEBUG)
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

void kdebug_dump_call_trace(PTR *raddrs, INT count)
{
    typedef struct stack_frame {
        struct stack_frame *ebp;
        unsigned int eip;
    } stack_frame;

    stack_frame *frame;
    __asm__ volatile( "mov %0, ebp;" : "=m" (frame));

    for (; count && frame->ebp; count--, frame = frame->ebp)
        *(raddrs++) = frame->eip;
}

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
/***************************************************************************************************
 * Prints log to the host console in a new line. When DEBUG is defined also prints the function name
 * and line number.
 *
 * @return      Nothing
 **************************************************************************************************/
void kdebug_log_ndu (KernelDebugLogType type, const char* func, UINT line, char* fmt, ...)
{
    int  len = 0;
    char buffer[MAX_PRINTABLE_STRING_LENGTH];

    switch (type)
    {
    case KDEBUG_LOG_TYPE_INFO:
    {
    #ifdef DEBUG
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\r\n  %s[ INFO ]%s %s:%u %s| ",
                               ANSI_COL_GREEN, ANSI_COL_GRAY, func, line, ANSI_COL_RESET);
    #else
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\r\n  %s[ INFO ]%s ", ANSI_COL_GREEN,
                               ANSI_COL_RESET);
    #endif // DEBUG
    }
    break;
    case KDEBUG_LOG_TYPE_ERROR:
    {
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\r\n  %s[ ERROR ]%s %s:%u %s| ",
                               ANSI_COL_RED, ANSI_COL_GRAY, func, line, ANSI_COL_RESET);
    }
    break;
    case KDEBUG_LOG_TYPE_FUNC:
    {
    #ifdef DEBUG
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\r\n%s[ %s:%u ]%s ", ANSI_COL_YELLOW,
                               func, line, ANSI_COL_RESET);
    #else
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\r\n%s[ %s ]%s ", ANSI_COL_YELLOW,
                               func, ANSI_COL_RESET);
    #endif // DEBUG
    }
    break;
    }

    va_list l;
    va_start (l, fmt);
    kearly_vsnprintf (buffer + len, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    s_qemu_debugPutString (buffer);
}
#endif
