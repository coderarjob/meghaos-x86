# Megha Operating System V2 - x86
----------------------------------------------------------------------------------------------------

## Virtual Memory Manager and Recursive mapping
categories: note, x86
_16 Dec 2022_

In order to make changes to a PDE or PTE, we require the virtual addresses of them. However, the PD
addresses in CR3 and PT address in a PDE are all physical.

Now, as there will always be one PD per process its virtual address can be part of the process
table. However, there can be thousands of page tables per process, storing virtual
address of each separately is going to be expensive - 4KB pre PD.

4KB does not sound too bad right, even with 100 processes, this will account to just 400KB. So I am
sure some OS has/will choose this route.

Having a separate table for the virtual addresses means, that the whole PD is available and so the
full 4 GB of virtual address space can be used.

MMURTL chooses to not have a separate table, but multipurpose the PD for this. It does this by
limiting the virtual address space of each process to 2GB, and uses the higher PD entries to store
the virtual addresses of pages tables (that are referenced in the PD).

One problem with storing virtual addresses of page tables separately is that now this table need to
be kept synchronised with the PD.

Recursive mapping is a much better.

### Recursive mapping

Here is how a virtual address to physical address map works - Going from MSB to LSB, the first 10
bits is index to select a PDE, the next 10 bits is index to select a PTE, the next 12 bits is used
to access each byte in the page.

In recursive mapping, the last PDE has the physical address of the PD instead of a PT. With this
simple change now, each PT in the PD now has a corresponding virtual address.

Here is how a virtual address to PT works - Going from MSB to LSB, the first 10 bits is index to
select a PDE (here it is always 0x3FF, select the 1024th PDE), the next 10 bits is index to select
a PDE (this is the PT we want to access), the next 12 bits is the offset into the page table (to
access PTEs in the PT).

With Higher mapped kernel, the kernel page tables can be accessed with virtual address in the range
`0xFFF00000 – FFF00FFC`.

Special case of this is when the second 10 bits is also 0x3FF, then the 12 bits becomes of the
offset into the PD and can be used to access PDEs - that is virtual address in the range
`FFFFF000 – FFFFFFFC` access PDEs in the PD.

----------------------------------------------------------------------------------------------------

## Map virtual memory to physical memory
categories: note, x86
_18 Dec 2022_

The functions in VMM for example (`kvmm_map`) will call functions in PMM. PMM and VMM are not
sibling modules, but have a parent child relationship.

By sibling I mean, one can ues PMM independently of VMM, this cannot be so - otherwise the
responsibility of keep PMM in sync with VMM will relay with the user. Instead physical page
allocation and deallocation must go though the interface presented by VMM.

kvmm_map which maps a Physical page to a Virtual one, will also call pmm_aloc if required, it should
not be called separately by the caller of kvmm_map.

```c
kvmm_map (Physical pa, uintptr_t pa, INT flags, INT access)
{
    // pa and va must be page aligned

    PageInfo info = s_getPagingInfo (va);

    if (!info.pde.present)
    {
        // Create a new page table
        Physical addr = kpmm_alloc (1, false);
        info.pde.page_table = addr

        // refresh!
        info = s_getPagingInfo (va);
    }

    if (info.pte.present)
        Error ("Virtual address already mapped to another physical address")
    else
        pte.page_frame = pa
}

kvmm_virtualToPhyical (uintptr_t va)
{
    PageInfo info = s_getPagingInfo (va);
    if (!info.pde.present || !info.pte.present)
        Error ("Not mapped");

     return createPhysical (info.pte.addr * PAGE_SIZE + info.offset);
}

PageInfo s_getPagingInfo (uintptr_t va)
{
    PageInfo info = {};

    info.pdeIndex = ((va & PDE_MASK) >> PDE_SHIFT)   // for kernel pages, this will be >= 768.
    info.pteIndex = ((va & PTE_MASK) >> PTE_SHIFT)
    info.offset = ((va & OFFSET_MASK) >> OFFSET_SHIFT)

    info.pde = (PDE*)((0x3FF << PDE_SHIT) | (0x3FF << PTE_SHIFT) | pdeIndex * sizeof(PDE))
    if (pde.present)
        info.pte = (PDE*)((0x3FF << PDE_SHIT) | (pdeIndex << PTE_SHIFT) | pteIndex * sizeof(PTE))

    return info;
}
```
