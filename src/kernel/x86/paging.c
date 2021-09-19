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

volatile u8 large_array[4096] = {100};

void paging_print()
{
    extern int g_page_dir, g_page_table;
    struct pde4kb *pde = (struct pde4kb *)(0xC0000000 + g_page_dir);
    struct pte    *pte = (struct pte *)   (0xC0000000 + g_page_table);
    
#define BIT1(t) ((t) & 0x1)
#define BIT20(t) ((t) & ((2 << 19) -1))

    printk("\r\nPage directory: %x\r\n", pde);
    printk("Page table    : %x\r\n", pte);

    printk("Dir 0: \r\n"
           "       page table addr = %x, \r\n"
           "       ps              = %x, \r\n"
           "       accessed        = %x, \r\n"
           "       cache disabled  = %x, \r\n"
           "       write-through   = %x, \r\n"
           "       user accessable = %x, \r\n"
           "       write allowed   = %x, \r\n"
           "       present         = %x, \r\n",
           BIT20(pde[0].pagetable_addr),
           BIT1(pde[0].ps_mustbe0),
           BIT1(pde[0].accessed),
           BIT1(pde[0].pcw),
           BIT1(pde[0].pwt),
           BIT1(pde[0].user_accessable),
           BIT1(pde[0].write_allowed),
           BIT1(pde[0].present));
                    
    printk("Dir 768: \r\n"
           "       page table addr = %x, \r\n"
           "       ps              = %x, \r\n"
           "       accessed        = %x, \r\n"
           "       cache disabled  = %x, \r\n"
           "       write-through   = %x, \r\n"
           "       user accessable = %x, \r\n"
           "       write allowed   = %x, \r\n"
           "       present         = %x, \r\n",
           BIT20(pde[768].pagetable_addr),
           BIT1(pde[768].ps_mustbe0),
           BIT1(pde[768].accessed),
           BIT1(pde[768].pcw),
           BIT1(pde[768].pwt),
           BIT1(pde[768].user_accessable),
           BIT1(pde[768].write_allowed),
           BIT1(pde[768].present));

    printk("Page B8: \r\n"
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
           BIT20(pte[1023].page_addr),
           BIT1(pte[1023].global_page),
           BIT1(pte[1023].pat),
           BIT1(pte[1023].dirty),
           BIT1(pte[1023].accessed),
           BIT1(pte[1023].pcw),
           BIT1(pte[1023].pwt),
           BIT1(pte[1023].user_accessable),
           BIT1(pte[1023].write_allowed),
           BIT1(pte[1023].present));
#undef BIT1
#undef BIT20

}
