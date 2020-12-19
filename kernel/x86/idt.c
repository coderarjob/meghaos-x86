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
    u16 offset_low;
    u16 segment_tss_selector;
    u8 zeros;
    u8 type:5;
    u8 dpl:2;
    u8 p:1;
    u16 offset_high;
} __attribute__((packed));

struct idt_size
{
    u16 limit;
    u32 location;
} __attribute__((packed));

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
static void idt_write();
/* -------------------------------------------------------------------------*/

/* Set all bytes to zero, for all the 256 IDT entries */
void kidt_init()
{
    idt = (struct idt_des*)INTEL_32_IDT_LOCATION;
    memset((void *)idt, 0, sizeof(struct idt_des) * 256);
    idt_write();
}

/* Edits an IDT descriptor */
void kidt_edit(int index, void(*func)() , u16 seg_tss_selector, 
        enum idt_des_types type, u8 dpl)
{
    u32 offset = (u32)func;

    idt[index].offset_low = offset & 0xFFFF;
    idt[index].offset_high = offset >> 16;
    idt[index].segment_tss_selector = seg_tss_selector;
    idt[index].type = type;    
    idt[index].dpl = dpl;
    idt[index].p = 1;
    idt[index].zeros = 0;
}

/* Writes the IDT structure address and length to the IDTR register.  */
static void idt_write()
{
    volatile struct idt_size s = {
        .limit = sizeof(struct idt_des) * 256 - 1,
        .location = (u32)idt
    };

    __asm__ ( "lidt [%0]"
             :
             :"a" (&s));
}
