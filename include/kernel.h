/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Kernel Header
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/
#ifndef __KERNEL_H__
#define __KERNEL_H__

#ifdef __linux__
#error "You are targetting Linux. Which is wrong!"
#endif 

#ifndef __i386__
#error "You are not targetting i386."
#endif

#include <types.h>
#include <stdarg.h>
#include <disp.h>
#include <mem.h>
#include <io.h>

#ifdef __i386__
    #include <x86/kernel.h> /* GDT, IO, MEMORY Addresses */
#endif

enum printk_types { PK_ONSCREEN };

/* Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

/* Printf like function, that prints depending on type*/
void printk(u8 type, const char *fmt, ...);

/* Prints formatted on screen at the cursor location.*/
void vprintk(const char *fmt, va_list list);

/* Displays an error message on the screen and Halts */
#define kpanic(s,...) __kpanic(__FILE__,__LINE__,s,__VA_ARGS__)
void __kpanic(const char *file, int lineno, const char *s,...);

#endif
