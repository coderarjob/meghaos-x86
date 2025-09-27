/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - IDT Header
* ---------------------------------------------------------------------------
*/
#ifndef IDT_H_X86
#define IDT_H_X86

#include <types.h>
#include <buildcheck.h>

typedef enum IDTDescriptorTypes
{
    IDT_DES_TYPE_TASK_GATE = 5,
    IDT_DES_TYPE_16_INTERRUPT_GATE = 6,
    IDT_DES_TYPE_32_INTERRUPT_GATE = 0xE,
    IDT_DES_TYPE_16_TRAP_GATE = 7,
    IDT_DES_TYPE_32_TRAP_GATE = 0xF
} IDTDescriptorTypes;

/* IDT Descriptor */
typedef struct IdtDescriptor IdtDescriptor;
typedef struct IdtMeta IdtMeta;

/* Fills the IDT table with zeros, and setup the IDTR register */
void kidt_init (void);

/* Edits an IDT descriptor */
void kidt_edit (INT                index,
                void             (*func)(void),
                U16                seg_selector,
                IDTDescriptorTypes type,
                U8                 dpl);

#endif //IDT_H_X86
