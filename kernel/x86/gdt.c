/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - GDT
*
* The GDT will reside in the kernel space and will be represented by the
* structure.
* ---------------------------------------------------------------------------
*
* Dated: 6th October 2020
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

// ASK: Is GDT not be aligned in any way?? 
__attribute__ ((aligned (8))) struct gdt_des gdt[5];

void gdt_init()
{
    gdt_segment_add(0,0,0,0,0);                         // NULL Segment
    gdt_segment_add(1,0,0xFFFFFFFF,0x9A,0xF);           // Code Segment
    gdt_segment_add(1,0,0xFFFFFFFF,0x92,0xF);           // Data Segment
}

static void gdt_write()
{
    struct gdt_size s = {.size = (u16)(sizeof(gdt)-1), 
                         .location = (u32)&gdt};
}

void gdt_segment_add(u8 gdt_index, u32 base, u32 limit, u8 access, u8 flags)
{
   gdt[gdt_index].limit_low = (u16)limit;
   gdt[gdt_index].base_low = (u16)base;
   gdt[gdt_index].base_middle = (u8)(base >> 16);
   gdt[gdt_index].access = access;
   gdt[gdt_index].limit_high = (limit>>16);
   gdt[gdt_index].flags = flags;
   gdt[gdt_index].base_high = (base >> 24);
}
