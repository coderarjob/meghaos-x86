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

#if defined(__linux__) && !defined(UNITTEST)
#error "You are targetting Linux. Which is wrong!"
#endif 

#if !defined(__i386__) && !(defined(UNITTEST) && ARCH == x86)
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
#include <moslimits.h>

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/kernel.h> /* GDT, IO, MEMORY Addresses */
#endif

/* Halts the processor by going into infinite loop */
#define k_halt() for (;;)

/* Used to know the offset of a member in a structure type */
#define offsetOf(type,member) ((size_t)(&((type *)0)->member))

/* Length of an array in bytes */
#define ARRAY_LENGTH(ar) (sizeof ((ar))/sizeof ((ar)[0]))

/* Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

/* Printf like function, that prints depending on type*/
INT kearly_printf (const CHAR *fmt, ...);

/* Writes formatted output to memory pointed to by the dest CHAR pointer.*/
INT kearly_snprintf (CHAR *dest, size_t size, const CHAR *fmt, ...);

/* Writes formatted output to memory pointed to by the dest CHAR pointer.*/
INT kearly_vsnprintf (CHAR *dest, size_t size, const CHAR *fmt, va_list l);

#endif
