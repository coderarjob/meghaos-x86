/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - GDT Header
* ---------------------------------------------------------------------------
*/
#ifndef GDT_H_X86
#define GDT_H_X86

#include <types.h>

typedef struct GdtDescriptor GdtDescriptor;
typedef struct GdtMeta GdtMeta;

#define GDT_SELECTOR_FROM_INDEX(index, ring) ((index)<<3)|(ring)

// Kernel Code segment selector.
#define GDT_INDEX_KCODE 1
#define GDT_SELECTOR_KCODE GDT_SELECTOR_FROM_INDEX (GDT_INDEX_KCODE, 0)

// Kernel Data segment selector.
#define GDT_INDEX_KDATA 2
#define GDT_SELECTOR_KDATA GDT_SELECTOR_FROM_INDEX (GDT_INDEX_KDATA, 0)

// Kernel TSS segment selector.
#define GDT_INDEX_KTSS  3
#define GDT_SELECTOR_KTSS GDT_SELECTOR_FROM_INDEX (GDT_INDEX_KTSS, 0)

// User Code segment selector.
#define GDT_INDEX_UCODE 4
#define GDT_SELECTOR_UCODE GDT_SELECTOR_FROM_INDEX (GDT_INDEX_UCODE, 3)

// User Data segment selector.
#define GDT_INDEX_UDATA 5
#define GDT_SELECTOR_UDATA GDT_SELECTOR_FROM_INDEX (GDT_INDEX_UDATA, 3)

/* Edits a GDT descriptor in the GDT table.
 * Note: If gdt_index < 3 or > gdt_count or > GDT_MAX_COUNT then an exception
 * is generated.
 */
void kgdt_edit (U16 gdt_index, U32 base, U32 limit, U8  access, U8  flags);

/* Writes the GDT structure address and length to the GDTR register.  */
void kgdt_write ();

#endif // GDT_H_X86
