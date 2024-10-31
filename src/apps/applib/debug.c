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
#include <applib/app.h>
#include <applib/syscall.h>

#if defined(DEBUG)
static void s_qemu_debugPutString (const CHAR* string)
{
    CHAR c;
    while ((c = *string++))
        outb (0xE9, c);
}

/***************************************************************************************************
 * Prints log to the host console in a new line. When DEBUG is defined also prints the function name
 * and line number.
 *
 * @return      Nothing
 **************************************************************************************************/
void debug_log_ndu (DebugLogType type, const char* func, UINT line, char* fmt, ...)
{
    int len = 0;
    char buffer[MAX_PRINTABLE_STRING_LENGTH];
    UINT tick_count = os_get_tickcount();

    switch (type) {
    case DEBUG_LOG_TYPE_INFO: {
        len = snprintf (buffer, ARRAY_LENGTH (buffer), "\n  %s[%u][ INFO ]%s %s:%u %s| ",
                        ANSI_COL_GREEN, tick_count, ANSI_COL_GRAY, func, line, ANSI_COL_RESET);
    } break;
    case DEBUG_LOG_TYPE_ERROR: {
        len = snprintf (buffer, ARRAY_LENGTH (buffer), "\n  %s[%u][ ERROR ]%s %s:%u %s| ",
                        ANSI_COL_RED, tick_count, ANSI_COL_GRAY, func, line, ANSI_COL_RESET);
    } break;
    case DEBUG_LOG_TYPE_FUNC: {
        len = snprintf (buffer, ARRAY_LENGTH (buffer), "\n%s[%u][ %s:%u ]%s ", ANSI_COL_YELLOW,
                        tick_count, func, line, ANSI_COL_RESET);
    } break;
    }

    va_list l;
    va_start (l, fmt);
    vsnprintf (buffer + len, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    s_qemu_debugPutString (buffer);
}
#endif // DEBUG
