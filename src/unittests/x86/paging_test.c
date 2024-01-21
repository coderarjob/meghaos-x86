#include <stdbool.h>
#include <types.h>
#include <paging.h>
#include <x86/paging.h>
#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/paging.h>
#include <mock/kernel/pmm.h>
#include <kerror.h>

KernelErrorCodes k_errorNumber;

// ------------------------------------------------------------------------------------------------
// Test: Temporary map and unmap scenarios
// ------------------------------------------------------------------------------------------------
TEST (paging, temporary_unmap_success)
{
    ArchPageDirectoryEntry pde     = { .present = 1 };
    s_getPdeFromCurrentPd_fake.ret = &pde;

    kpg_temporaryUnmap();
    EQ_SCALAR ((int)pde.present, 0);
    END();
}

TEST (paging, temporary_unmap_failure_already_unmapped)
{
    ArchPageDirectoryEntry pde     = { 0 };
    s_getPdeFromCurrentPd_fake.ret = &pde;

    kpg_temporaryUnmap();
    EQ_SCALAR (panic_invoked, true);
    EQ_SCALAR ((int)pde.present, 0);
    END();
}

TEST (paging, temporary_map_success)
{
    ArchPageDirectoryEntry pde     = { 0 };
    s_getPdeFromCurrentPd_fake.ret = &pde;

    Physical pa = PHYSICAL (0x12000); // 0x12000 is just a valid physical address.
    kpg_temporaryMap (pa);

    EQ_SCALAR ((U32)pde.pageTableFrame, PHYSICAL_TO_PAGEFRAME (pa.val));
    EQ_SCALAR ((U32)pde.present, 1);

    END();
}

TEST (paging, temporary_map_failure_already_mapped)
{
    ArchPageDirectoryEntry pde     = { .present = 1 };
    s_getPdeFromCurrentPd_fake.ret = &pde;

    Physical pa = PHYSICAL (0x12000);
    kpg_temporaryMap (pa);

    EQ_SCALAR (panic_invoked, true);

    END();
}

TEST (paging, temporary_map_failure_input_not_aligned)
{
    ArchPageDirectoryEntry pde     = { 0 };
    s_getPdeFromCurrentPd_fake.ret = &pde;

    Physical pa = PHYSICAL (0x12); // Address is not page aligned.
    EQ_SCALAR ((PTR)NULL, (PTR)kpg_temporaryMap (pa));
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    END();
}
// ------------------------------------------------------------------------------------------------

TEST (paging, get_currentpd_success)
{
    ArchPageDirectoryEntry pde     = { .pageTableFrame = 0xFFBB }; // Some magic number that's all.
    s_getPdeFromCurrentPd_fake.ret = &pde;
    EQ_MEM (kpg_getcurrentpd(), &pde, sizeof (PageDirectory));
    END();
}

// ------------------------------------------------------------------------------------------------
// Test: Unsuccessful mapping scenarios
// ------------------------------------------------------------------------------------------------

TEST (paging, map_failure_double_allocate)
{
    // Note:
    // Since we do not have physical addresses, we mock them in the following ways:
    // 1. Use virtual addresses.
    // 2. Use any valid number.

    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present = 1 }  // PDE which changes as result of map. Here setup to show already mapped.
    };

    // Page directory with three entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 1. (To indicate page table exists).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 1 }, // Page table exists
        { .present = 0 }  // PDE which is used for temporary map.
    };

    // 1. We want the PDE entry at index 2 for temporary map, so we set it up as follows.
    // 1. Present bit is 0 (To indicate that nothing is already mapped).
    // 2. Return virtual address of this PD entry for temporary map.
    s_getPdeFromCurrentPd_fake.ret = &pd[2];

    // Since we are not working with physical addresses in the test we do not require the temporary
    // mapping of the Page Table physical address to get a temporary virtual address (we already
    // have a working virtual address for the Page Table). This is done in the following way:
    // 1. Return the virtual address of this PT for the map operation.
    s_getPteFromCurrentPd_fake.ret = pt;

    EQ_SCALAR (kpg_map (pd, 0, va, pa), false);

    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_ALLOC);

    END();
}

TEST (paging, map_failure_new_page_table_creation_failed)
{
    // Note:
    // Since we do not have physical addresses, we mock them in the following ways:
    // 1. Use virtual addresses.
    // 2. Use any valid number.

    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }  // PDE which changes as result of map.
    };

    // Page directory with three entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 0. (To indicate page table absent).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }, // Page table does not exists. kpg_map should create a new one.
        { .present = 0 }  // PDE which is used for temporary map.
    };

    // Physical address for the new page table
    kpmm_alloc_fake.ret = false;

    EQ_SCALAR (kpg_map (pd, 0, va, pa), false);
    EQ_SCALAR (panic_invoked, true);

    END();
}

TEST (paging, map_failure_va_not_aligned)
{
    PTR                    va = 0xC01FF001;         // Any misaligned virtual address.
    Physical               pa = PHYSICAL (0x12000); // Any Page Aligned number will work.
    ArchPageDirectoryEntry pd = { 0 };

    EQ_SCALAR (kpg_map (&pd, 0, va, pa), false);

    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    END();
}

TEST (paging, map_failure_pa_not_aligned)
{
    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12001);                      // Any Page Aligned number will work.
    ArchPageDirectoryEntry pd = { 0 };

    EQ_SCALAR (kpg_map (&pd, 0, va, pa), false);

    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    END();
}

TEST (paging, map_failure_pd_is_null)
{
    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.
    EQ_SCALAR (kpg_map (NULL, 0, va, pa), false);

    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);

    END();
}

// ------------------------------------------------------------------------------------------------
// Test: Successful mapping scenarios
// ------------------------------------------------------------------------------------------------
bool kpmm_alloc_handler_map_success_page_table_not_present (Physical *address, UINT pageCount,
                                                            KernelPhysicalMemoryRegions reg)
{
    address->val = 0x13000; // Can be Page aligned any  number.
    return true;
}

TEST (paging, map_success_page_table_not_present)
{
    // Note:
    // Since we do not have physical addresses, we mock them in the following ways:
    // 1. Use virtual addresses.
    // 2. Use any valid number.

    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }  // PDE which changes as result of map.
    };

    // Page directory with three entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 0. (To indicate page table absent).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }, // Page table does not exists. kpg_map should create a new one.
        { .present = 0 }  // PDE which is used for temporary map.
    };

    // Physical address for the new page table
    kpmm_alloc_fake.handler = kpmm_alloc_handler_map_success_page_table_not_present;

    // 1. We want the PDE entry at index 2 for temporary map, so we set it up as follows.
    // 1. Present bit is 0 (To indicate that nothing is already mapped).
    // 2. Return virtual address of this PD entry for temporary map.
    s_getPdeFromCurrentPd_fake.ret = &pd[2];

    // Since we are not working with physical addresses in the test we do not require the temporary
    // mapping of the Page Table physical address to get a temporary virtual address (we already
    // have a working virtual address for the Page Table). This is done in the following way:
    // 1. Return the virtual address of this PT for the map operation.
    s_getPteFromCurrentPd_fake.ret = pt;

    EQ_SCALAR (kpg_map (pd, 0, va, pa), true);

    // New page table should be added to PDE at index 1. We should expect the following:
    // 1. Preset bit is set.
    // 2. Page frame should point to the physical address of the new page table. That in this case
    // will be equal to the value in the PD entry used for temporary map.
    EQ_SCALAR ((U32)pd[1].present, 1);
    EQ_SCALAR ((U32)pd[1].pageTableFrame, (U32)pd[2].pageTableFrame);

    // After kpg_map, the entry at index 1 should have the Page frame of the physical address
    // (here value in 'pa') and the present bit set.
    EQ_SCALAR ((U32)pt[1].present, 1);
    EQ_SCALAR ((U32)pt[1].pageFrame, PHYSICAL_TO_PAGEFRAME (pa.val));

    END();
}

TEST (paging, map_success_page_table_present)
{
    // Note:
    // Since we do not have physical addresses, we mock them in the following ways:
    // 1. Use virtual addresses.
    // 2. Use any valid number.

    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }  // PDE which changes as result of map.
    };

    // Page directory with three entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 1. (To indicate page table exists).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 1 }, // Page table exists
        { .present = 0 }  // PDE which is used for temporary map.
    };

    // 1. We want the PDE entry at index 2 for temporary map, so we set it up as follows.
    // 1. Present bit is 0 (To indicate that nothing is already mapped).
    // 2. Return virtual address of this PD entry for temporary map.
    s_getPdeFromCurrentPd_fake.ret = &pd[2];

    // Since we are not working with physical addresses in the test we do not require the temporary
    // mapping of the Page Table physical address to get a temporary virtual address (we already
    // have a working virtual address for the Page Table). This is done in the following way:
    // 1. Return the virtual address of this PT for the map operation.
    s_getPteFromCurrentPd_fake.ret = pt;

    EQ_SCALAR (kpg_map (pd, 0, va, pa), true);

    // After kpg_map, the entry at index 1 should have the Page frame of the physical address
    // (here value in 'pa') and the present bit set.
    EQ_SCALAR ((U32)pt[1].present, 1);
    EQ_SCALAR ((U32)pt[1].pageFrame, PHYSICAL_TO_PAGEFRAME (pa.val));

    END();
}

// ------------------------------------------------------------------------------------------------
// Test: Unmapping virtual addresses
// ------------------------------------------------------------------------------------------------

TEST (paging, unmap_success)
{
    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present   = 1,
          .pageFrame = PHYSICAL_TO_PAGEFRAME (pa.val) } // PDE which changes as result of map.
    };

    // Page directory with three entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 1. (To indicate page table exists).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 1 }, // Page table exists
        { .present = 0 }  // PDE which is used for temporary map.
    };

    // 1. We want the PDE entry at index 2 for temporary map, so we set it up as follows.
    // 1. Present bit is 0 (To indicate that nothing is already mapped).
    // 2. Return virtual address of this PD entry for temporary map.
    s_getPdeFromCurrentPd_fake.ret = &pd[2];

    // Since we are not working with physical addresses in the test we do not require the temporary
    // mapping of the Page Table physical address to get a temporary virtual address (we already
    // have a working virtual address for the Page Table). This is done in the following way:
    // 1. Return the virtual address of this PT for the unmap operation.
    s_getPteFromCurrentPd_fake.ret = pt;

    EQ_SCALAR (kpg_unmap (pd, va), true);

    // After the unmap the present bit should get unset.
    EQ_SCALAR ((U32)pt[1].present, 0);

    END();
}

TEST (paging, unmap_failure_page_table_not_present)
{
    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }  // PDE already unmapped.
    };

    // Page directory with two entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 0. (To indicate page table not present).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }, // Page table exists
    };

    EQ_SCALAR (kpg_unmap (pd, va), false);

    EQ_SCALAR (panic_invoked, true);

    END();
}

TEST (paging, unmap_failure_double_unmap)
{
    PTR      va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    Physical pa = PHYSICAL (0x12000);                      // Any Page Aligned number will work.

    // Page Table requires at-least two entries.
    __attribute__ ((aligned (4096))) ArchPageTableEntry pt[] = {
        { .present = 0 }, // Not used.
        { .present = 0 }  // PDE already unmapped.
    };

    // Page directory with three entries. As per the choice of virtual address here (value in 'va')
    // entry at index 1 is required to have the following setup:
    // 1. Preset bit set to 1. (To indicate page table exists).
    __attribute__ ((aligned (4096))) ArchPageDirectoryEntry pd[] = {
        { .present = 0 }, // Not used.
        { .present = 1 }, // Page table exists
        { .present = 0 }  // PDE which is used for temporary map.
    };

    // 1. We want the PDE entry at index 2 for temporary map, so we set it up as follows.
    // 1. Present bit is 0 (To indicate that nothing is already mapped).
    // 2. Return virtual address of this PD entry for temporary map.
    s_getPdeFromCurrentPd_fake.ret = &pd[2];

    // Since we are not working with physical addresses in the test we do not require the temporary
    // mapping of the Page Table physical address to get a temporary virtual address (we already
    // have a working virtual address for the Page Table). This is done in the following way:
    // 1. Return the virtual address of this PT for the unmap operation.
    s_getPteFromCurrentPd_fake.ret = pt;

    EQ_SCALAR (kpg_unmap (pd, va), false);

    // After the unmap the present bit should get unset.
    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_FREE);

    END();
}

TEST (paging, unmap_failure_va_not_aligned)
{
    PTR                    va = 0xC01FF001; // Any misaligned virtual address.
    ArchPageDirectoryEntry pd = { 0 };

    EQ_SCALAR (kpg_unmap (&pd, va), false);

    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    END();
}

TEST (paging, unmap_failure_pd_is_null)
{
    PTR va = (0x1 << PDE_SHIFT) | (0x1 << PTE_SHIFT); // PD[1] & PT[1] will be used.
    EQ_SCALAR (kpg_unmap (NULL, va), false);

    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);

    END();
}

// ------------------------------------------------------------------------------------------------

void reset()
{
    panic_invoked = false;
    resetPagingFake();
    resetMemFake();
    resetPmm();
}

int main()
{
    temporary_unmap_success();
    temporary_unmap_failure_already_unmapped();

    temporary_map_success();
    temporary_map_failure_input_not_aligned();
    temporary_map_failure_already_mapped();

    get_currentpd_success();

    map_success_page_table_present();
    map_success_page_table_not_present();
    map_failure_pd_is_null();
    map_failure_new_page_table_creation_failed();
    map_failure_pa_not_aligned();
    map_failure_va_not_aligned();
    map_failure_double_allocate();

    unmap_success();
    unmap_failure_va_not_aligned();
    unmap_failure_pd_is_null();
    unmap_failure_double_unmap();
    unmap_failure_page_table_not_present();

    return 0;
}
