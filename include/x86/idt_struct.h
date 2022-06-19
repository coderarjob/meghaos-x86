/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - IDT abstract structures
* ---------------------------------------------------------------------------
*/
#ifndef IDT_STRUCT_H_X86
#define IDT_STRUCT_H_X86

#include <kernel.h>

/* IDT Descriptor */
struct IdtDescriptor
{
    U16 offset_low;
    U16 segment_tss_selector;
    U8  zeros;
    U8  type : 5;
    U8  dpl  : 2;
    U8  p    : 1;
    U16 offset_high;
} __attribute__ ((packed));

struct IdtMeta
{
    U16 limit;
    U32 location;
} __attribute__ ((packed));

#endif //IDT_STRUCT_H_X86
