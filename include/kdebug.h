/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Printing to debug console.
 * ---------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <buildcheck.h>
#include <stdarg.h>

typedef enum KernelDebugLogType {
    KDEBUG_LOG_TYPE_INFO,
    KDEBUG_LOG_TYPE_FUNC,
    KDEBUG_LOG_TYPE_ERROR,
    KDEBUG_LOG_TYPE_WARN,
} KernelDebugLogType;

/* Prints formatted string to 0xE9 port and can optionally print to vga
 * buffer.
 */

#if defined(PORT_E9_ENABLED)
    void kdebug_log_ndu (KernelDebugLogType type, const char* func, UINT line, char* fmt, ...);

    #define INFO(...) kdebug_log_ndu (KDEBUG_LOG_TYPE_INFO, __func__, __LINE__, __VA_ARGS__)

    #define FUNC_ENTRY(...) \
        kdebug_log_ndu (KDEBUG_LOG_TYPE_FUNC, __func__, __LINE__, "" __VA_ARGS__)
    #define ERROR(...) kdebug_log_ndu (KDEBUG_LOG_TYPE_ERROR, __func__, __LINE__, __VA_ARGS__)
    #define WARN(...)  kdebug_log_ndu (KDEBUG_LOG_TYPE_WARN, __func__, __LINE__, __VA_ARGS__)

#else
    #define INFO(...)       (void)0
    #define FUNC_ENTRY(...) (void)0
    #define ERROR(...)      (void)0
    #define WARN(...)       (void)0
#endif

#if defined(DEBUG)
    #if !defined(GRAPHICS_MODE_ENABLED)
        typedef enum DisplayControls {
            DISP_GETCOORDS,
            DISP_SETCOORDS,
            DISP_COORDS_MAX,
            DISP_SETATTR,
            DISP_GETATTR
        } DisplayControls;

        INT kdisp_ioctl (INT request, ...);
        void kdisp_scrollDown();
        void kdisp_init();
        void kdisp_putc (CHAR c);
    #endif
    INT kearly_printf (const CHAR* fmt, ...);
#else
    #define kearly_printf(...) (void)0
#endif // DEBUG

INT kearly_snprintf (CHAR* dest, size_t size, const CHAR* fmt, ...);
INT kearly_vsnprintf (CHAR* dest, size_t size, const CHAR* fmt, va_list l);

/***************************************************************************************************
 * Moves to the next line and prints formatted input on the screen.
 *
 * @return      For description for `kearly_vsnprintf`.
 **************************************************************************************************/
#define kearly_println(...) kearly_printf ("\n" __VA_ARGS__)

/***************************************************************************************************
 * Magic break point used by bochs emulator
 *
 * @return      Nothing
 **************************************************************************************************/
#define kbochs_breakpoint() __asm__ volatile("xchg bx, bx")

// TODO: Should all bugs be fatal?
#define BUG()               ERROR ("\n\nBUG: %s:%u", __FILE__, __LINE__);
#define FATAL_BUG()         k_panic ("\n\nBUG: %s:%u", __FILE__, __LINE__);

#define WARN_ON(t, ...)                                          \
    do {                                                         \
        if (!(t)) {                                              \
            WARN ("\n\nWARN ON: (" #t ") failed\n" __VA_ARGS__); \
        }                                                        \
    } while (0)

#define BUG_ON(t, ...) \
    do {               \
        if (!(t)) {    \
            BUG();     \
        }              \
    } while (0)
