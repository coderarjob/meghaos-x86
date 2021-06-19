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

/* Prints an ASCII character based on kdebug_putc implementation.
 * When debugging with Qemu, default implementation is to output
 * to 0xE9 port.
 */
void kdebug_putc(char c);

#endif // __KDEBUG_H__
