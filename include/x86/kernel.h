/*
 * External Kernel routines, used by the kernel. These routines are not system
 * calls and may not be accessible by user mode programs.
 *
 * Note:
 * Remember that these header files are for building OS and its utilitites, it
 * is not a SDK.
 * */

#ifndef __KERNEL_H_x86__
#define __KERNEL_H_x86__

#include <ints.h>
#include <x86/mem.h>

#define VGA_TEXT_WHITE  0xF
#define VGA_TEXT_GREY   0x7
#define VGA_TEXT_RED    0x4

/* Halts the processor by going into infinite loop */
#define khalt() __asm__ volatile ("1: jmp 1;")

/* Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

void kprintf(const char *format, ...);
/*
 * Prints an ASCII character on the VGA text mode frame buffer
 * and increments the pointer to it.
 * */
void kputc(char c, u8 attribute);

/*
 * Prints a ASCIZ string on the VGA text mode frame buffer
 * and increments the pointer to it.
 * */
void kputs(char *s, u8 attribute);

/* Writes a new or overrides an existing GDT entry 
 * Causes a exception if gdt_index = 0.
 * */
void kgdt_add(u8 gdt_index, u32 base, u32 limit, u8 access, u8 flags);
#endif //__KERNEL_H_x86__
