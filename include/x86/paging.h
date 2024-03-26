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

#ifndef PAGING_H_X86
#define PAGING_H_X86

#include <types.h>
#include <buildcheck.h>


#define PDE_SHIFT    22U
#define PTE_SHIFT    12U
#define OFFSET_SHIFT 00U

#define PDE_MASK    0xFFC00000U
#define PTE_MASK    0x003FF000U
#define OFFSET_MASK 0x00000FFFU

#define RECURSIVE_PDE_INDEX 1023U
#define KERNEL_PDE_INDEX    768U
#define TEMPORARY_PTE_INDEX 1023U

#define LINEAR_ADDR(pde_idx, pte_idx, offset) \
    (((pde_idx) << PDE_SHIFT) | ((pte_idx) << PTE_SHIFT) | (offset))

#ifndef UNITTEST
    /***********************************************************************************************
     * Invalidates TLB entries specific to a virtual address.
     *
     * @Input   addr    Virtual address
     * @return          Nothing
     **********************************************************************************************/
    #define x86_TLB_INVAL_SINGLE(addr) __asm__ volatile("invlpg %0;" ::"m"(*(char*)addr))

    #define x86_LOAD_CR3(pd) __asm__ volatile("mov CR3, %0;" ::"r"(pd))

    /***********************************************************************************************
     * Invalidates complete TLB.
     *
     * @return          Nothing
     **********************************************************************************************/
    #define X86_TLB_INVAL_COMPLETE()                  \
        do                                            \
        {                                             \
            int temp;                                 \
            __asm__ volatile("mov %0, cr3; "          \
                             "mov cr3, %0;"           \
                             : "=r"(temp)::"memory"); \
        } while (0)
#else
    #define x86_TLB_INVAL_SINGLE(addr) (void)0
    #define X86_TLB_INVAL_COMPLETE()   (void)0
#endif

/* Casts a linear mapped physical address to virtual address */
static inline void* CAST_PA_TO_VA (Physical a)
{
#if !defined(UNITTEST)
    return (void *)(0xC0000000 + a.val);
#else
    return (void *)a.val;
#endif
}

extern Physical g_page_dir,     /* Address of the initial page dir */
                g_page_table;   /* Address of the initial page table*/

/* 4 KByte Page table entry */
struct ArchPageTableEntry
{
    UINT    present              : 1,
            write_allowed        : 1,
            user_accessable      : 1,
            write_through_cache  : 1,
            cache_disabled       : 1,
            accessed             : 1,
            dirty                : 1,
            page_attribute_table : 1,
            global_page          : 1,
            ignore               : 3,
            pageFrame            :20;
} __attribute__ ((packed));

/* 4 KByte Page Directory entry */
struct ArchPageDirectoryEntry
{
    UINT    present              : 1,
            write_allowed        : 1,
            user_accessable      : 1,
            write_through_cache  : 1,
            cache_disabled       : 1,
            accessed             : 1,
            ignore0              : 1,
            ps_mustbe0           : 1,
            ignore1              : 4,
            pageTableFrame       :20;
} __attribute__ ((packed));

#endif // PAGING_H_X86
