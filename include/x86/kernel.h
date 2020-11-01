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

#include <x86/mem.h>
#include <x86/io.h>

/* Halts the processor by going into infinite loop */
#define khalt() __asm__ volatile ("jmp $;");

/* Magic break point used by bochs emulator*/
#define kbochs_breakpoint() __asm__ volatile ("xchg bx, bx")

/* Edits a GDT descriptor in the GDT table.
 * Note: If gdt_index < 3 then an exception is generated.  */
void kgdt_edit(u16 gdt_index, u32 base, u32 limit, u8 access, u8 flags);

/* Writes the GDT structure address and length to the GDTR register.  */
void kgdt_write();
#endif //__KERNEL_H_x86__
