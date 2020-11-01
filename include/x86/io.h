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

#ifndef __IO_H__
#define __IO_H__

#define outb(p,v) __asm__ volatile ("out %%dx, %%al;" : :"a" (v), "d" (p))
#define inb(p,v)  __asm__ volatile ("in %%al, %%dx" :"=a" (v) :"d" (p))

#endif // __IO_H__
