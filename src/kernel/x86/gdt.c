/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - GDT
*
* The GDT will reside in the kernel space and will be represented by the
* structure.
* ---------------------------------------------------------------------------
* Dated: 6th October 2020
; ---------------------------------------------------------------------------
*/

#include <types.h>
#include <moslimits.h>
#include <x86/gdt.h>
#include <x86/gdt_struct.h>
#include <kdebug.h>
#include <x86/memloc.h>
#include <panic.h>

/* -------------------------------------------------------------------------*/
/* Variables */
static volatile GdtDescriptor *s_gdt;
static          U16            s_gdt_count = MIN_GDT_INDEX;
/* -------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------*/
/* Function definations */

/* Writes the GDT structure address and length to the GDTR register.  */
void
kgdt_write ()
{
    volatile GdtMeta gdt_size_and_loc =
    {
        .size     = (U16)(sizeof (GdtDescriptor) * s_gdt_count - 1),
        .location = (U32)s_gdt
    };

    kdebug_printf("GDT {size = 0x%x, location = 0x%x}",
                   gdt_size_and_loc.size, gdt_size_and_loc.location);

    // NOTE: No need to load the SS, DS, ES or CS registers, as it already
    // contains the values needs (from boo1)
    __asm__ ( "lgdt [%0]"
             :
             :"a" (&gdt_size_and_loc));
}

/* Edits a GDT descriptor in the GDT table.
 * Note: If gdt_index < 3 or > s_gdt_count or > GDT_MAX_COUNT then an panic is
 * generated.
 */
void
kgdt_edit (U16 gdt_index,
           U32 base,
           U32 limit,
           U8  access,
           U8  flags)
{ 
    s_gdt = (GdtDescriptor *)INTEL_32_GDT_LOCATION;

    // Valid range is MIN_INDEX < index < s_gdt_count < GDT_MAX_COUNT
    if (!(gdt_index >= MIN_GDT_INDEX &&
          gdt_index <= s_gdt_count   &&
          gdt_index <= MAX_GDT_DESC_COUNT))
          k_panic ("Invalid gdt_index: %u",gdt_index);

    s_gdt[gdt_index].limit_low   = limit & 0xFFFF;
    s_gdt[gdt_index].limit_high  = (limit >> 16) & 0xF;
    s_gdt[gdt_index].base_low    = base & 0xFFFF;
    s_gdt[gdt_index].base_middle = (U8)(base >> 16) & 0xFF;
    s_gdt[gdt_index].base_high   = (U8)(base >> 24) & 0xFF;
    s_gdt[gdt_index].access      = access;
    s_gdt[gdt_index].flags       = (U8)(flags & 0xF);

    // Check if a new GDT entry has been added or an old one is editted.
    // On next kgdt_write, gdtr.size will be increase.
    if (gdt_index == s_gdt_count)
        s_gdt_count++;
}
/* -------------------------------------------------------------------------*/
