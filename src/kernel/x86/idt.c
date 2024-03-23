/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - IDT
*
* The IDT will reside in the kernel space and will be represented by the
* structure.
* ---------------------------------------------------------------------------
* Dated: 27th November 2020
*/

#include <x86/idt.h>
#include <kstdlib.h>
#include <x86/memloc.h>
#include <moslimits.h>
#include <kdebug.h>
#include <x86/idt_struct.h>

/* -------------------------------------------------------------------------*/
/* Variables */

#define IDT_BYTES  sizeof(IdtDescriptor) * MAX_IDT_DESC_COUNT
static IdtDescriptor *s_idt;
/* -------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------*/
/* Local Function */
static void s_idt_write ();
/* -------------------------------------------------------------------------*/

/* Set all bytes to zero, for all the 256 IDT entries */
void
kidt_init ()
{
    FUNC_ENTRY();

    s_idt = (IdtDescriptor *)INTEL_32_IDT_LOCATION;
    k_memset ((void *)s_idt, 0, IDT_BYTES);
    s_idt_write ();
}

/* Edits an IDT descriptor */
void
kidt_edit (INT                index,
           void             (*func)(),
           U16                seg_selector,
           IDTDescriptorTypes type,
           U8                 dpl)
{
    FUNC_ENTRY ("index: 0x%x, func: 0x%px, seg_sel: 0x%x, type: 0x%x, dpl: 0x%x", index, func,
                seg_selector, type, dpl);

    U32 offset = (U32)func;

    s_idt[index].offset_low           = offset & 0xFFFF;
    s_idt[index].offset_high          = (U16)(offset >> 16) & 0xFFFF;
    s_idt[index].segment_selector     = seg_selector;
    s_idt[index].type                 = type;
    s_idt[index].dpl                  = (U8)(dpl & 0x3);
    s_idt[index].p                    = 1;
    s_idt[index].zeros                = 0;
}

/* Writes the IDT structure address and length to the IDTR register.  */
static void
s_idt_write ()
{
    volatile IdtMeta idt_size_and_loc =
    {
        .limit = IDT_BYTES - 1,
        .location = (U32)s_idt
    };

    INFO ("IDT {limit = 0x%x, location = 0x%x}", idt_size_and_loc.limit, idt_size_and_loc.location);

    __asm__ ( "lidt [%0]"
             :
             :"a" (&idt_size_and_loc));
}
