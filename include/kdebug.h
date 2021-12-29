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
#ifndef __KDEBUG_H__
#define __KDEBUG_H__

#include <types.h>

/* Prints formatted string to 0xE9 port and can optionally print to vga
 * buffer.
 */
#if defined (DEBUG)
    void kdebug_printf_ndu (const CHAR *fmt, ...);
    #define kdebug_printf(fmt,...) kdebug_printf_ndu (fmt,__VA_ARGS__)
#else
    #define kdebug_printf(fmt,...) (void)0
#endif

#endif // __KDEBUG_H__
