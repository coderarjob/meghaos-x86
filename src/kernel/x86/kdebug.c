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
#include <kernel.h>

#if defined(DEBUG) && defined(PORT_E9_ENABLED)
static void s_qemu_debugPutString (const CHAR *string)
{
    CHAR c;
    while ((c = *string++))
        outb (0xE9, c);
}

void kdebug_printf_ndu (const CHAR *fmt, ...)
{
    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start (l,fmt);
    kearly_vsnprintf (buffer, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    // Print to E9 port 
    s_qemu_debugPutString (buffer);
}

/***************************************************************************************************
 * Prints log to the host console in a new line. When DEBUG is defined also prints the function name
 * and line number.
 *
 * @return      Nothing
 **************************************************************************************************/
void kdebug_log_ndu (KernelDebugLogType type, const char* func, UINT line, char* fmt, ...)
{
    int len = 0;
    char buffer[MAX_PRINTABLE_STRING_LENGTH];

    switch (type) {
    case KDEBUG_LOG_TYPE_INFO: {
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\n  %s[%u][ INFO ]%s %s:%u %s| ",
                               ANSI_COL_GREEN, g_kstate.tick_count, ANSI_COL_GRAY, func, line,
                               ANSI_COL_RESET);
    } break;
    case KDEBUG_LOG_TYPE_ERROR: {
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\n  %s[%u][ ERROR ]%s %s:%u %s| ",
                               ANSI_COL_RED, g_kstate.tick_count, ANSI_COL_GRAY, func, line,
                               ANSI_COL_RESET);
    } break;
    case KDEBUG_LOG_TYPE_FUNC: {
        len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\n%s[%u][ %s:%u ]%s ",
                               ANSI_COL_YELLOW, g_kstate.tick_count, func, line, ANSI_COL_RESET);
    } break;
    }

    va_list l;
    va_start (l, fmt);
    kearly_vsnprintf (buffer + len, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    s_qemu_debugPutString (buffer);
}
#endif // DEBUG && PORT_E9_ENABLED

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
