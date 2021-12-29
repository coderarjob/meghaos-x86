/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - IDT
*
* The IDT will reside in the kernel space and will be represented by the
* structure.
* ---------------------------------------------------------------------------
* Dated: 27th November 2020
; ---------------------------------------------------------------------------
; Bulid 20201127
; - Initial version.
; ---------------------------------------------------------------------------
*/

#include <kernel.h>

struct idt_des
{
    U16 offset_low;
    U16 segment_tss_selector;
    U8  zeros;
    U8  type : 5;
    U8  dpl  : 2;
    U8  p    : 1;
    U16 offset_high;
} __attribute__ ((packed));

struct idt_size
{
    U16 limit;
    U32 location;
} __attribute__ ((packed));

/* -------------------------------------------------------------------------*/
/* Variables */

/* TODO: 
 * Following assignment which sets idt = 0, is getting ignored by the compiler.
 * That is the reason, we are doing the same assignment in kidt_init.
 */

//static struct idt_des *idt = (struct idt_des*)INTEL_32_IDT_LOCATION;
static struct idt_des *idt;

/* -------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------*/
/* Local Function */
static void idt_write ();
/* -------------------------------------------------------------------------*/

/* Set all bytes to zero, for all the 256 IDT entries */
void kidt_init ()
{
    idt = (struct idt_des*)INTEL_32_IDT_LOCATION;
    k_memset ((void *)idt, 0, sizeof (struct idt_des) * 256);
    idt_write ();
}

/* Edits an IDT descriptor */
void kidt_edit (INT                index,
                void             (*func)(),
                U16                seg_tss_selector,
                IDTDescriptorTypes type,
                U8                 dpl)
{
    U32 offset = (U32)func;

    idt[index].offset_low           = offset & 0xFFFF;
    idt[index].offset_high          = (U16)(offset >> 16) & 0xFFFF;
    idt[index].segment_tss_selector = seg_tss_selector;
    idt[index].type                 = type;
    idt[index].dpl                  = (U8)(dpl & 0x3);
    idt[index].p                    = 1;
    idt[index].zeros                = 0;
}

/* Writes the IDT structure address and length to the IDTR register.  */
static void idt_write ()
{
    volatile struct idt_size s = {
        .limit = sizeof (struct idt_des) * 256 - 1,
        .location = (U32)idt
    };

    __asm__ ( "lidt [%0]"
             :
             :"a" (&s));
}
