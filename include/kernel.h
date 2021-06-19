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

#if __STDC_VERSION__ >= 199901
    #define __C99__
#endif

#ifdef __linux__
#error "You are targetting Linux. Which is wrong!"
#endif 

#ifndef __i386__
#error "You are not targetting i386."
#endif

#include <types.h>
#include <stdarg.h>
#include <disp.h>
#include <kdebug.h>
#include <mem.h>
#include <io.h>
#include <panic.h>
#include <assert.h>
#include <errno.h>

#ifdef __i386__
    #include <x86/kernel.h> /* GDT, IO, MEMORY Addresses */
#endif

enum printk_types { PK_ONSCREEN, PK_DEBUG };

/* Halts the processor by going into infinite loop */
#define khalt() for(;;)

/* Used to know the offset of a member in a structure type */
#define OFFSET_OF(type,member) ((size_t)(&((type *)0)->member))

/* Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

/* Printf like function, that prints depending on type*/
void printk(u8 type, const char *fmt, ...);

/* Prints formatted on screen at the cursor location.*/
void vprintk(u8 type, const char *fmt, va_list list);

#endif
