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

/* Prints formatted string to 0xE9 port and can optionally print to vga
 * buffer.
 */
#if defined (DEBUG) && !defined (UNITTEST)
    void kdebug_printf_ndu (const CHAR *fmt, ...);
    #define kdebug_printf(...) kdebug_printf_ndu (__VA_ARGS__)
#else
    #define kdebug_printf(fmt,...) (void)0
#endif

/***************************************************************************************************
 * Moves to the next line and prints formatted input on the screen and E9 port.
 *
 * @return      Nothing
 **************************************************************************************************/
#define kdebug_println(...) kdebug_printf("\r\n" __VA_ARGS__)

/***************************************************************************************************
 * Magic break point used by bochs emulator
 *
 * @return      Nothing
 **************************************************************************************************/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")
void kdebug_dump_call_trace(PTR *raddrs, INT count);

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void kdebug_log_ndu (const char* type, const char* funcname, UINT linenumber, char* fmt, ...);

    #define INFO(...)  kdebug_log_ndu ("Info", __func__, __LINE__, __VA_ARGS__)
    #define WARN(...)  kdebug_log_ndu ("Warn", __func__, __LINE__, __VA_ARGS__)
    #define ERROR(...) kdebug_log_ndu ("Error", __func__, __LINE__, __VA_ARGS__)
#else
    #define INFO(...)  (void)0
    #define WARN(...)  (void)0
    #define ERROR(...) (void)0
#endif // DEBUG_LEVEL & 1

#endif // KDEBUG_H
