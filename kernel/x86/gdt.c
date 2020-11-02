/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - GDT
*
* The GDT will reside in the kernel space and will be represented by the
* structure.
* ---------------------------------------------------------------------------
* Dated: 6th October 2020
; ---------------------------------------------------------------------------
; Bulid 20201019
; - DT now resides at a static location set by boot1
; - boot1 program sets up the kernel GDT and is fully ready. No longer do the 
;   kernel needs re-initialize the GDT.
; - GDT resides at location 0x0800 (right after the last IDT entry)
; ---------------------------------------------------------------------------
; Bulid 20201008
; - Initial version with Load GDT complete.
; ---------------------------------------------------------------------------
*/

#include <kernel.h>

#define GDT_MIN_INDEX   3     // Minimum index that can be editted in Kernel.
#define GDT_COUNT       512   // Number of GDT entries in memory

struct gdt_des
{
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 limit_high:4;
    u8 flags     :4;
    u8 base_high;
} __attribute__((packed));

struct gdt_size
{
    u16 size;
    u32 location;
} __attribute__((packed));

/* -------------------------------------------------------------------------*/
/* Variables */
static struct gdt_des *gdt = (struct gdt_des*)INTEL_32_GDT_LOCATION;
/* -------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------*/
/* Function definations */

/* Writes the GDT structure address and length to the GDTR register.  */
void kgdt_write()
{
    struct gdt_size s = {.size = (u16)(sizeof(gdt)-1), 
                         .location = (u32)&gdt};

    // NOTE: No need to load the SS, DS, ES or CS registers, as it already
    // contains the values needs (from boo1)
    __asm__ ( "lgdt [%0]"
             :
             :"a" (&s));
}

/* Edits a GDT descriptor in the GDT table.
 * Note: If gdt_index < 3 then an exception is generated.  */
void kgdt_edit(u16 gdt_index, u32 base, u32 limit, u8 access, u8 flags)
{
    if (gdt_index < GDT_MIN_INDEX || 
        gdt_index >= GDT_COUNT) {
        kpanic("Invalid gdt_index. GDT_MIN_INDEX: %xH, gdt_index: %xH, "
                "GDT_COUNT: %xH", GDT_MIN_INDEX,gdt_index,GDT_COUNT);
    }

    gdt[gdt_index].limit_low = (u16)limit;
    gdt[gdt_index].base_low = (u16)base;
    gdt[gdt_index].base_middle = (u8)(base >> 16);
    gdt[gdt_index].access = access;
    gdt[gdt_index].limit_high = (limit>>16);
    gdt[gdt_index].flags = flags;
    gdt[gdt_index].base_high = (base >> 24);
}
/* -------------------------------------------------------------------------*/
