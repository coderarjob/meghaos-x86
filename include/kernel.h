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
#include <config.h>
#include <interrupt.h>

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/kernel.h> /* GDT, IO, MEMORY Addresses */
#endif

/* Halts the processor by going into infinite loop */
#define k_halt() for (;;)

/** Used to know the offset of a member in a structure type */
#define offsetOf(type,member) ((size_t)(&((type *)0)->member))

/** Length of an array in bytes */
#define ARRAY_LENGTH(ar) (sizeof ((ar))/sizeof ((ar)[0]))

/** Checks if 'a' is aligned to 'n' */
#define IS_ALIGNED(a, n) (((a) & ((n) - 1)) == 0)

/* If 'a' is not already aligned to 'n', returns next aligned number */
#define ALIGN_UP(a, n) (IS_ALIGNED((a),(n)) ? (a)                   \
                                            : (((a) / (n)) + 1) * (n))

/** If 'a' is not already aligned to 'n', returns previous aligned number */
#define ALIGN_DOWN(a, n) (IS_ALIGNED((a),(n)) ? (a)                   \
                                              : ((a) / (n))  * (n))

/** Maximum of two integers. If equal, returns 'b' */
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/** Minimum of two integers. If equal, returns 'b' */
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/** Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

/** Stringfy macros */
#define STR_NDU(v) #v
#define STR(v) STR_NDU(v)

INT kearly_printf (const CHAR *fmt, ...);
INT kearly_snprintf (CHAR *dest, size_t size, const CHAR *fmt, ...);
INT kearly_vsnprintf (CHAR *dest, size_t size, const CHAR *fmt, va_list l);

#endif
