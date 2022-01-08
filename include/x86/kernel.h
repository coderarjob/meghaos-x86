/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Contains function
* declerations and includes header files necessory for building x86 Kernel.
* These routines are not system calls and may not be accessible by user mode 
* programs.
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#ifndef __KERNEL_H_x86__
#define __KERNEL_H_x86__

#include <x86/boot.h>
#include <x86/paging.h>

typedef enum IDTDescriptorTypes 
{
    IDT_DES_TYPE_TASK_GATE = 5, 
    IDT_DES_TYPE_16_INTERRUPT_GATE = 6,
    IDT_DES_TYPE_32_INTERRUPT_GATE = 0xE,
    IDT_DES_TYPE_16_TRAP_GATE = 7,
    IDT_DES_TYPE_32_TRAP_GATE = 0xF
} IDTDescriptorTypes;

#define __GDT_SELECTOR_FROM_INDEX(index, ring) ((index)<<3)|(ring)

// Kernel Code segment selector.
#define GDT_INDEX_KCODE 1
#define GDT_SELECTOR_KCODE __GDT_SELECTOR_FROM_INDEX (GDT_INDEX_KCODE, 0)

// Kernel Data segment selector.
#define GDT_INDEX_KDATA 2
#define GDT_SELECTOR_KDATA __GDT_SELECTOR_FROM_INDEX (GDT_INDEX_KDATA, 0)

// Kernel TSS segment selector.
#define GDT_INDEX_KTSS  3
#define GDT_SELECTOR_KTSS __GDT_SELECTOR_FROM_INDEX (GDT_INDEX_KTSS, 0)

// User Code segment selector.
#define GDT_INDEX_UCODE 4
#define GDT_SELECTOR_UCODE __GDT_SELECTOR_FROM_INDEX (GDT_INDEX_UCODE, 3)

// User Data segment selector.
#define GDT_INDEX_UDATA 5
#define GDT_SELECTOR_UDATA __GDT_SELECTOR_FROM_INDEX (GDT_INDEX_UDATA, 3)

/* Initializes the tss_entry structure, installs a tss segment in GDT */
void ktss_init  ();

/* Edits a GDT descriptor in the GDT table.
 * Note: If gdt_index < 3 or > gdt_count or > GDT_MAX_COUNT then an exception 
 * is generated.
 */
void kgdt_edit (U16 gdt_index,
                U32 base,
                U32 limit,
                U8  access,
                U8  flags);

/* Writes the GDT structure address and length to the GDTR register.  */
void kgdt_write ();

/* Fills the IDT table with zeros, and setup the IDTR register */
void kidt_init ();

/* Edits an IDT descriptor */
void kidt_edit (INT                index,
                void             (*func)(),
                U16                seg_tss_selector,
                IDTDescriptorTypes type,
                U8                 dpl);

#endif //__KERNEL_H_x86__
