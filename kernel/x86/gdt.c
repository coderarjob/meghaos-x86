/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - GDT
*
* The GDT will reside in the kernel space and will be represented by the
* structure.
* ---------------------------------------------------------------------------
* Dated: 6th October 2020
; ---------------------------------------------------------------------------
; Bulid 20201104
; - panic is caused if gdt_index < MIN_INDEX or > gdt_count or > MAX_COUNT
; - gdt_count local var, tracks the current number of gdt entries.
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
static volatile struct gdt_des *gdt = (struct gdt_des*)INTEL_32_GDT_LOCATION;
static u16 gdt_count = GDT_MIN_INDEX;

/* -------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------*/
/* Function definations */

/* Writes the GDT structure address and length to the GDTR register.  */
void kgdt_write()
{
    volatile struct gdt_size s = {
        .size = sizeof(struct gdt_des) * gdt_count -1,
        .location = (u32)gdt
    };

    // NOTE: No need to load the SS, DS, ES or CS registers, as it already
    // contains the values needs (from boo1)
    __asm__ ( "lgdt [%0]"
             :
             :"a" (&s));
}

/* Edits a GDT descriptor in the GDT table.
 * Note: If gdt_index < 3 or > gdt_count or > GDT_MAX_COUNT then an exception 
 * is generated.  */
void kgdt_edit(u16 gdt_index, u32 base, u32 limit, u8 access, u8 flags)
{
    // Valid range is MIN_INDEX < index < gdt_count < GDT_MAX_COUNT
    if (gdt_index < GDT_MIN_INDEX || 
        gdt_index > gdt_count || 
        gdt_index > GDT_MAX_COUNT) {
        kpanic("Invalid gdt_index. MIN_INDEX: %xH, index: %xH, "
                "gdt_count: %xH, GDT_COUNT: %xH", 
                GDT_MIN_INDEX,
                gdt_index,
                gdt_count,
                GDT_MAX_COUNT);
    }

    gdt[gdt_index].limit_low = (u16)limit;
    gdt[gdt_index].base_low = (u16)base;
    gdt[gdt_index].base_middle = (u8)(base >> 16);
    gdt[gdt_index].access = access;
    gdt[gdt_index].limit_high = (limit>>16);
    gdt[gdt_index].flags = flags;
    gdt[gdt_index].base_high = (base >> 24);

    // Check is a new GDT has been added or an old one is editted.
    // Increment gdt_count and gdtr.size accordingly
    if (gdt_index == gdt_count)
        gdt_count++;
}
/* -------------------------------------------------------------------------*/
