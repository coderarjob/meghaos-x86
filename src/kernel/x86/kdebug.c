/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Debug console
 *
 * Uses the port 0xE9 hack to output characters to the linux Qemu console.
 * ---------------------------------------------------------------------------
 */
#include <stdarg.h>
#include <types.h>
#include <moslimits.h>
#include <kdebug.h>
#include <kernel.h>
#if ARCH == x86
    #include <x86/io.h>
#endif

#define ANSI_COL_GRAY   "\x1b[90m"
#define ANSI_COL_YELLOW "\x1b[93m"
#define ANSI_COL_RED    "\x1b[31m"
#define ANSI_COL_GREEN  "\x1b[32m"
#define ANSI_COL_RESET  "\x1b[0m"

#if defined(DEBUG) && defined(PORT_E9_ENABLED)
static void s_qemu_debugPutString (const CHAR* string)
{
    CHAR c;
    while ((c = *string++))
        outb (0xE9, c);
}

void kdebug_printf_ndu (const CHAR* fmt, ...)
{
    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start (l, fmt);
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
    char* message  = NULL;
    char* logColor = ANSI_COL_RESET;

    switch (type) {
    case KDEBUG_LOG_TYPE_INFO: {
        message  = "\n  %s[%u][ INFO ]%s %s:%u %s|";
        logColor = ANSI_COL_GREEN;
    } break;
    case KDEBUG_LOG_TYPE_FUNC: {
        message  = "\n%s[%u]%s[ %s:%u ]%s|";
        logColor = ANSI_COL_YELLOW;
    } break;
    case KDEBUG_LOG_TYPE_ERROR: {
        message  = "\n  %s[%u][ ERROR ]%s %s:%u %s|";
        logColor = ANSI_COL_RED;
    } break;
    case KDEBUG_LOG_TYPE_WARN: {
        message  = "\n  %s[%u][ WARN ]%s %s:%u %s|";
        logColor = ANSI_COL_YELLOW;
    } break;
    }

    len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), message, logColor, g_kstate.tick_count,
                           ANSI_COL_GRAY, func, line, ANSI_COL_RESET);

    va_list l;
    va_start (l, fmt);
    kearly_vsnprintf (buffer + len, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    s_qemu_debugPutString (buffer);
}
#endif // DEBUG && PORT_E9_ENABLED
