/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Kernel and usermode paging
* structures and casting methods.
*
* Manages all the page directories and page tables used by the kernel for its
* use as well as when allocating memory for user space processes.
* ---------------------------------------------------------------------------
* Dated: 13 September 2021
; ---------------------------------------------------------------------------
*/

#include <kernel.h>

#ifndef __PAGING_H_X86__
#define __PAGING_H_X86__

extern PHYSICAL_ADDRESS g_page_dir,     /* Address of the initial page dir */
                        g_page_table;   /* Address of the initial page table*/

/* Casts a linear mapped physical address to virtual address */
inline void* CAST_PA_VA(PHYSICAL_ADDRESS a)
{
    return (void *)(0xC0000000 + a.value);
}

/* 4 KByte Page table entry */
struct pte
{
    int    present          : 1,
           write_allowed    : 1,
           user_accessable  : 1,
           pwt              : 1,
           pcw              : 1,
           accessed         : 1,
           dirty            : 1,
           pat              : 1,
           global_page      : 1,
           ing2             : 3,
           page_addr        :20;
} __attribute__((packed));

/* 4 MByte Page Directory entry */
struct pde4mb
{
    int    pagetable_low    :10,
           zeros            : 5,
           pagetable_high   : 4,
           pat              : 1;
    int    ing2             : 3,
           global           : 1,
           ps_mustbe1       : 1,
           dirty            : 1,
           accessed         : 1,
           pcw              : 1,
           pwt              : 1,
           user_accessable  : 1,
           write_allowed    : 1,
           present          : 1;
} __attribute__((packed));

/* 4 KByte Page Directory entry */
struct pde4kb
{
    int    present          : 1,
           write_allowed    : 1,
           user_accessable  : 1,
           pwt              : 1,
           pcw              : 1,
           accessed         : 1,
           ing0             : 1,
           ps_mustbe0       : 1,
           ing2             : 4,
           pagetable_addr   :20;
} __attribute__((packed));

#endif // __PAGING_H_X86__
