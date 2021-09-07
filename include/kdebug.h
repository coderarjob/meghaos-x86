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
#if defined(DEBUG)
    void printk_debug_gs(const char *fmt, ...);
    #define printk_debug(fmt,...) printk_debug_gs(fmt,__VA_ARGS__)
#else
    #define printk_debug(fmt,...) (void)0
#endif

#endif // __KDEBUG_H__
