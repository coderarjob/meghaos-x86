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
#include <cm/cm.h>
#include <cm/syscall.h>
#include <cm/debug.h>

#if defined(DEBUG) && defined(PORT_E9_ENABLED)

    #define MAX_PRINTABLE_STRING_LENGTH 100
    #define ARRAY_LENGTH(ar)            (sizeof ((ar)) / sizeof ((ar)[0]))

    #define ANSI_COL_GRAY               "\x1b[90m"
    #define ANSI_COL_YELLOW             "\x1b[93m"
    #define ANSI_COL_RED                "\x1b[31m"
    #define ANSI_COL_GREEN              "\x1b[32m"
    #define ANSI_COL_RESET              "\x1b[0m"

static void qemu_putString (const CHAR* string)
{
    CHAR c;
    while ((c = *string++)) {
    #if ARCH == x86
        cm_debug_x86_outb (0xE9, c);
    #else
        #error "Not implemented"
    #endif
    }
}

/***************************************************************************************************
 * Prints log to the host console in a new line. When DEBUG is defined also prints the function name
 * and line number.
 *
 * @return      Nothing
 **************************************************************************************************/
void cm_debug_log_ndu (CM_DebugLogType type, const char* func, UINT line, char* fmt, ...)
{
    int len = 0;
    char buffer[MAX_PRINTABLE_STRING_LENGTH];
    char* message  = NULL;
    char* logColor = ANSI_COL_RESET;

    switch (type) {
    case CM_DEBUG_LOG_TYPE_INFO: {
        message  = "\n  %s[%u][MLC][ INFO ]%s %s:%u %s|";
        logColor = ANSI_COL_GREEN;
    } break;
    case CM_DEBUG_LOG_TYPE_FUNC: {
        message  = "\n%s[%u][MLC]%s[ %s:%u ]%s|";
        logColor = ANSI_COL_YELLOW;
    } break;
    case CM_DEBUG_LOG_TYPE_ERROR: {
        message  = "\n  %s[%u][MLC][ ERROR ]%s %s:%u %s|";
        logColor = ANSI_COL_RED;
    } break;
    case CM_DEBUG_LOG_TYPE_WARN: {
        message  = "\n  %s[%u][MLC][ WARN ]%s %s:%u %s|";
        logColor = ANSI_COL_YELLOW;
    } break;
    }

    UINT tick_count = cm_get_tickcount();
    len = snprintf (buffer, ARRAY_LENGTH (buffer), message, logColor, tick_count, ANSI_COL_GRAY,
                    func, line, ANSI_COL_RESET);

    va_list l;
    va_start (l, fmt);
    vsnprintf (buffer + len, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    qemu_putString (buffer);
}
#endif // DEBUG
