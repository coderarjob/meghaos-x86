/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Printing to debug 
* console.
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/
#ifndef KDEBUG_H
#define KDEBUG_H

#include <types.h>
#include <buildcheck.h>
#include <disp.h>

#define ANSI_COL_GRAY   "\x1b[90m"
#define ANSI_COL_YELLOW "\x1b[93m"
#define ANSI_COL_RED    "\x1b[31m"
#define ANSI_COL_GREEN  "\x1b[32m"
#define ANSI_COL_RESET  "\x1b[0m"

typedef enum KernelDebugLogType
{
    KDEBUG_LOG_TYPE_INFO,
    KDEBUG_LOG_TYPE_ERROR,
    KDEBUG_LOG_TYPE_FUNC
} KernelDebugLogType;

/* Prints formatted string to 0xE9 port and can optionally print to vga
 * buffer.
 */
#if defined (DEBUG) && !defined (UNITTEST)
    void kdebug_printf_ndu (const CHAR *fmt, ...);
    #define kdebug_printf(...) kdebug_printf_ndu (__VA_ARGS__)
#else
    #define kdebug_printf(...) (void)0
#endif

/***************************************************************************************************
 * Moves to the next line and prints formatted input on the screen and E9 port.
 *
 * @return      Nothing
 **************************************************************************************************/
#define kdebug_println(...) kdebug_printf("\n" __VA_ARGS__)

/***************************************************************************************************
 * Magic break point used by bochs emulator
 *
 * @return      Nothing
 **************************************************************************************************/
#define kbochs_breakpoint() __asm__ volatile("xchg bx, bx")
void kdebug_dump_call_trace (PTR* raddrs, INT count);

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
    void kdebug_log_ndu (KernelDebugLogType type, const char* func, UINT line, char* fmt, ...);

    #define INFO(...)  kdebug_log_ndu (KDEBUG_LOG_TYPE_INFO, __func__, __LINE__, __VA_ARGS__)
    #define ERROR(...) kdebug_log_ndu (KDEBUG_LOG_TYPE_ERROR, __func__, __LINE__, __VA_ARGS__)
    #define FUNC_ENTRY(...) \
        kdebug_log_ndu (KDEBUG_LOG_TYPE_FUNC, __func__, __LINE__, "Args: " __VA_ARGS__)
#else
    #define INFO(...)       (void)0
    #define ERROR(...)      (void)0
    #define FUNC_ENTRY(...) (void)0
#endif // DEBUG_LEVEL & 1

#define WARN_ON(t, ...)                                                          \
    do {                                                                         \
        if (!(t)) {                                                              \
            kdisp_importantPrint ("\n\nWARN ON: (" #t ") failed\n" __VA_ARGS__); \
        }                                                                        \
    } while (0)

// TODO: Should all bugs be fatal?
#define BUG()       kdisp_importantPrint ("\n\nBUG: %s:%u", __FILE__, __LINE__);

#define FATAL_BUG() k_panic ("\n\nBUG: %s:%u", __FILE__, __LINE__);

#endif // KDEBUG_H
