/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - App library - General macros & function declarations
 * ---------------------------------------------------------------------------
 *
 *  This file contains Mos-libc macros & definitions for application to use.
 */

#pragma once

#include <types.h>
#include <stdarg.h>

#define INVALID_HANDLE              -1
#define MAX_PRINTABLE_STRING_LENGTH 100

/** Length of an array in bytes */
#define ARRAY_LENGTH(ar)            (sizeof ((ar)) / sizeof ((ar)[0]))

#define HALT()                      for (;;)

#if ARCH == x86
    #define outb(p, v) __asm__ volatile("out dx, al;" : : "a"(v), "d"(p))
    #define inb(p, v)  __asm__ volatile("in al, dx" : "=a"(v) : "d"(p))
#endif

INT snprintf (CHAR* dest, size_t size, const CHAR* fmt, ...);
INT vsnprintf (CHAR* dest, size_t size, const CHAR* fmt, va_list l);

/********************************************************************
 * Debug specific. Only available for Debug builds of Mos-libc
 *******************************************************************/
#if defined(DEBUG) && defined(PORT_E9_ENABLED)
    typedef enum DebugLogType {
        DEBUG_LOG_TYPE_INFO,
        DEBUG_LOG_TYPE_FUNC,
        DEBUG_LOG_TYPE_ERROR,
        DEBUG_LOG_TYPE_WARN,
    } DebugLogType;

    void debug_log_ndu (DebugLogType type, const char* func, UINT line, char* fmt, ...);

    #define INFO(...)       debug_log_ndu (DEBUG_LOG_TYPE_INFO, __func__, __LINE__, __VA_ARGS__)
    #define ERROR(...)      debug_log_ndu (DEBUG_LOG_TYPE_ERROR, __func__, __LINE__, __VA_ARGS__)
    #define FUNC_ENTRY(...) debug_log_ndu (DEBUG_LOG_TYPE_FUNC, __func__, __LINE__, "" __VA_ARGS__)
    #define WARN(...)       debug_log_ndu (DEBUG_LOG_TYPE_WARN, __func__, __LINE__, __VA_ARGS__)
#else
    #define INFO(...)       (void)0
    #define ERROR(...)      (void)0
    #define FUNC_ENTRY(...) (void)0
    #define WARN(...)       (void)0
#endif // PORT_E9_ENABLED

#if defined(DEBUG)
    /***************************************************************************************************
     * Magic break point used by bochs emulator
     *
     * @return      Nothing
     **************************************************************************************************/
    #define bochs_breakpoint() __asm__ volatile("xchg bx, bx")
#endif // DEBUG

/***************************************************************************************************
 * Halts thread for 'ms' miliseconds
 *
 * @return      Nothing
 **************************************************************************************************/
void delay (UINT ms);
