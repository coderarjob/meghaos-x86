/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - GDT abstract structures
* ---------------------------------------------------------------------------
*/
#ifndef GDT_STRUCT_H_X86
#define GDT_STRUCT_H_X86

#include <types.h>

struct GdtDescriptor
{
    U16 limit_low;
    U16 base_low;
    U8 base_middle;
    U8 access;
    U8 limit_high:4;
    U8 flags     :4;
    U8 base_high;
} __attribute__ ((packed));

struct GdtMeta
{
    U16 size;
    U32 location;
} __attribute__ ((packed));

#endif // GDT_STRUCT_H_X86
