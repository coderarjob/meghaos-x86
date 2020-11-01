
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

#ifdef __i386__
    #include <x86/kernel.h> /* GDT, IO, MEMORY Addresses */
#endif

enum printk_types { PK_ONSCREEN };

/* Printf like function, that prints depending on type*/
void printk(u8 type, const char *fmt, ...);

/* Prints formatted on screen at the cursor location.*/
void vprintk(const char *fmt, va_list list);

#endif
