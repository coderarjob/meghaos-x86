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
#if defined (DEBUG)
    void kdebug_printf_ndu (const CHAR *fmt, ...);
    #define kdebug_printf(fmt,...) kdebug_printf_ndu (fmt,__VA_ARGS__)
#else
    #define kdebug_printf(fmt,...) (void)0
#endif

/** Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

void kdebug_dump_call_trace(PTR *raddrs, INT count);

#endif // KDEBUG_H
