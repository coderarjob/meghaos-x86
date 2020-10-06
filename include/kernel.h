/*
 * External Kernel routines, used by the kernel. These routines are not system
 * calls and may not be accessible by user mode programs.
 *
 * Note:
 * Remember that these header files are for building OS and its utilitites, it
 * is not a SDK.
 * */

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <ints.h>
/*
 * Displays a 16 bit number in Hexadecimal representation on the screen.
 */
void kprinthex(u16 num);

/*
 * Prints a ASCIZ string on the VGA text mode frame buffer.
 */
void prints(char *s);

void gdt_init();
void gdt_segment_add(u8 gdt_index, u32 base, u32 limit, u8 access, u8 flags);
#endif //__KERNEL_H__
