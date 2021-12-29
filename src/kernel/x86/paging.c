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
           "       write - through   = %x, \r\n"
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
           "       write - through   = %x, \r\n"
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
           "       write - through   = %x, \r\n"
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
