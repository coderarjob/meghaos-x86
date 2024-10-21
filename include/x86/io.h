/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Basic I/O routines.
*
* This is not and never going to be a full featured VGA text mode driver.
* Functions here will be limited to prints or printf like functions.
* NOTE: Actual VGA driver will be a server in User Mode.
* ---------------------------------------------------------------------------
*
* Dated: 24th October 2020
*/

#ifndef IO_H_X86
#define IO_H_X86

#include <buildcheck.h>
#include <types.h>

#define outb(p,v) __asm__ volatile ("out dx, al;" : :"a" (v), "d" (p))
#define inb(p,v)  __asm__ volatile ("in al, dx" :"=a" (v) :"d" (p))
// Reference: https://wiki.osdev.org/Inline_Assembly/Examples#IO_WAIT
#define io_delay() outb(0x80, 0)

static inline U8 ioread (INT port)
{
    U8 v;
    inb (port, v);
    return v;
}

#endif // IO_H_X86
