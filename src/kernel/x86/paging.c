/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Kernel and usermode paging routines
*
* Manages all the page directories and page tables used by the kernel for its
* use as well as when allocating memory for user space processes.
* ---------------------------------------------------------------------------
* Dated: 13 September 2021
; ---------------------------------------------------------------------------
*/
#include <kernel.h>

void display_PageInfo ()
{
#define MASK(b) (~((1 << b) - 1))

#define PDE_SHIFT   22
#define PTE_SHIFT   12
#define OFFSET_SHIFT 0

#define PDE_MASK    0xFFC00000
#define PTE_MASK    0x003FF000
#define OFFSET_MASK 0x00000FFF

    typedef struct PageInfo
    {
        UINT pdeIndex;
        UINT pteIndex;
        UINT offset;
        PageDirectoryEntry4KB *pde;
        PageTableEntry *pte;
    } PageInfo;

    uintptr_t va = 0xC0400004;

    PageInfo info = {0};
    info.pdeIndex = (va & PDE_MASK) >> PDE_SHIFT;
    info.pteIndex = (va & PTE_MASK) >> PTE_SHIFT;
    info.offset = (va & OFFSET_MASK) >> OFFSET_SHIFT;

    kdebug_printf ("\r\npde index: %x\r\npte index: %x\r\noffset: %x\r\n",
                    info.pdeIndex, info.pteIndex, info.offset);

    uintptr_t pdeAddr = (0x3FFU << PDE_SHIFT) |
                        (0x3FFU << PTE_SHIFT) |
                        (info.pdeIndex * sizeof(PageDirectoryEntry4KB));

    kdebug_printf ("\r\npdeAddr: %lx", pdeAddr);

    uintptr_t pteAddr = (0x3FFU        << PDE_SHIFT) |
                        (info.pdeIndex << PTE_SHIFT) |
                        (info.pteIndex * sizeof(PageTableEntry));

    kdebug_printf ("\r\npteAddr: %lx", pteAddr);

    info.pde = (PageDirectoryEntry4KB*)(pdeAddr);
    if (info.pde->present)                  // Check if a page table address is present in the pde
        info.pte = (PageTableEntry*)(pteAddr);

    if (info.pte && info.pte->present)      // Check if a page frame address is present in the pte
    {
        Physical paddr = createPhysical (info.pte->page_addr * CONFIG_PAGE_FRAME_SIZE_BYTES + info.offset);
        kdebug_printf ("\r\n%lx --> %lx", va, paddr.val);
    }
    else
        kdebug_printf ("\r\n%lx --> unmapped", va);

}

void paging_print ()
{
    PageDirectoryEntry4KB *pde = (PageDirectoryEntry4KB *)CAST_PA_TO_VA (g_page_dir);
    PageTableEntry    *pte = (PageTableEntry *)   CAST_PA_TO_VA (g_page_table);

#define CAST_BIT1_TO_U32(t)  CAST_BITN_TO_U32 (t,  1)
#define CAST_BIT20_TO_U32(t) CAST_BITN_TO_U32 (t, 20)

    kearly_printf ("\r\nPage directory: %x\r\n", pde);
    kearly_printf (    "Page table    : %x\r\n", pte);

    kearly_printf ("Dir 0: \r\n"
           "       page table addr = %x, \r\n"
           "       ps              = %x, \r\n"
           "       accessed        = %x, \r\n"
           "       cache disabled  = %x, \r\n"
           "       write-through   = %x, \r\n"
           "       user accessable = %x, \r\n"
           "       write allowed   = %x, \r\n"
           "       present         = %x, \r\n",
           CAST_BIT20_TO_U32 (pde[0].pagetable_addr),
           CAST_BIT1_TO_U32 (pde[0].ps_mustbe0),
           CAST_BIT1_TO_U32 (pde[0].accessed),
           CAST_BIT1_TO_U32 (pde[0].pcw),
           CAST_BIT1_TO_U32 (pde[0].pwt),
           CAST_BIT1_TO_U32 (pde[0].user_accessable),
           CAST_BIT1_TO_U32 (pde[0].write_allowed),
           CAST_BIT1_TO_U32 (pde[0].present));
                    
    kearly_printf ("Dir 768: \r\n"
           "       page table addr = %x, \r\n"
           "       ps              = %x, \r\n"
           "       accessed        = %x, \r\n"
           "       cache disabled  = %x, \r\n"
           "       write-through   = %x, \r\n"
           "       user accessable = %x, \r\n"
           "       write allowed   = %x, \r\n"
           "       present         = %x, \r\n",
           CAST_BIT20_TO_U32 (pde[768].pagetable_addr),
           CAST_BIT1_TO_U32 (pde[768].ps_mustbe0),
           CAST_BIT1_TO_U32 (pde[768].accessed),
           CAST_BIT1_TO_U32 (pde[768].pcw),
           CAST_BIT1_TO_U32 (pde[768].pwt),
           CAST_BIT1_TO_U32 (pde[768].user_accessable),
           CAST_BIT1_TO_U32 (pde[768].write_allowed),
           CAST_BIT1_TO_U32 (pde[768].present));

    kearly_printf ("Page 1023: \r\n"
           "       page addr       = %x, \r\n"
           "       global          = %x, \r\n"
           "       pat             = %x, \r\n"
           "       dirty           = %x, \r\n"
           "       accessed        = %x, \r\n"
           "       cache disabled  = %x, \r\n"
           "       write-through   = %x, \r\n"
           "       user accessable = %x, \r\n"
           "       write allowed   = %x, \r\n"
           "       present         = %x, \r\n",
           CAST_BIT20_TO_U32 (pte[1023].page_addr),
           CAST_BIT1_TO_U32 (pte[1023].global_page),
           CAST_BIT1_TO_U32 (pte[1023].pat),
           CAST_BIT1_TO_U32 (pte[1023].dirty),
           CAST_BIT1_TO_U32 (pte[1023].accessed),
           CAST_BIT1_TO_U32 (pte[1023].pcw),
           CAST_BIT1_TO_U32 (pte[1023].pwt),
           CAST_BIT1_TO_U32 (pte[1023].user_accessable),
           CAST_BIT1_TO_U32 (pte[1023].write_allowed),
           CAST_BIT1_TO_U32 (pte[1023].present));
#undef CAST_BIT1_TO_U32
#undef CAST_BIT20_TO_U32

}
