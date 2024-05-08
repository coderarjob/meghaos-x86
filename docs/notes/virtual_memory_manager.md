# Megha Operating System V2 - x86
----------------------------------------------------------------------------------------------------

## Virtual memory management - Concept of Regions
categories: note, x86
_6 April 2024_

Regions cover small (say 20 pages in size) continuous virtual addresses and keep track of virtual &
physical page allocations/deallocations within its address space. Region combine the PMM and paging
to provide a wholesome virtual & physical memory management.

As regions contains a small number of pages, the tracking can be done using a 2 bit bitmap (0 -
unallocated, 1 - committed, 2 - uncommitted).

Without Region, the following becomes a problem:
1. To unmap virtual pages together with their physical counterparts. It is better to do these atomic
   operations in one place rather than spread across in multiple places.
2. With on-demand physical page allocation, subsequent 'find free virtual page' operation may
   return same virtual page (as the virtual page is not yet committed). Regions provide a solution;
   at the time of region creation, it can mark uncommitted pages and not allocate them again for
   another region.

Which means that through Regions, we will get our VMM.

Note that the same information which can be stored in bitmaps can also be stored in Page tables and
get the same benefits of 'late commit' of physical pages and 'uncommitted allocation'. However this
route will have some disadvantages.

1. For storing allocation data into Page tables, Page tables must exist. So this means even for
   uncommitted allocation, page tables must be created so that the data about the allocation can be
   stored there. The benefit of late commitment is reduced and becomes more prominent when
   allocations that spans multiple page tables.
2. Much slower than scanning a bitmap.
3. Care must be taken so the magic number used to indicate 'uncommitted allocation' and
   'unallocated' pages never occurs naturally.
4. Architecture and paging structure depth dependent. For example moving from 2-level to 4-level
   paging may required extensive changes to the virtual page allocation logic.

----------------------------------------------------------------------------------------------------

## Why I am skipping Virtual Memory manager for now
categories: note, x86
_16 Dec 2022_

From what I have read and understood, the VMM's does the following:
1. Like the physical memory manager, virtual memory manager also has as AllocPages function. It
   looks for free virtual pages. It might not allocate physical memory, but just mark those pages as
   allocated.
2. Related to the above point is the `map` function of VMM. It keeps note of the allocated VAs which
   the OS will use later when allocating physical pages. I could be that this is a sub function of
   the `AllocPages` function.
3. This simplifies some aspects of mapping, where we can over-map VMM and not worry about physical
   memory usage.
4. VMM manages `regions/address spaces`, which are a portion of VA assigned for a purpose (Kernel
   heap has one region, Static allocator has another region, Stack has another region etc). One can
   then ask to expand these `region` and VMM will make sure that we are crashing into another
   region.
5. Provides data structure to quickly search the VA-PA mapping. Provided a VA we can search which PA
   is assigned to it. We can parse page tables, but it will be arch dependent and slow.
6. VMM may also automatically assign a VA range when region gets created. Currently we are planning
   and hard-coding each region by hand.

Though all of the above points are important, they are not crucial or blocking. With a `Static
allocator` and `Heap allocator` ready, I can proceed with other parts of the OS and come back to
implement VMM at a later time. I think I will be able to appreciate VMM better once I see the
problem it is solving and also I am kind of bored with the VMM at this time.

Note however that in the absence of a VMM we have to do the following:
1. We have to use pre-planned and hard-coded start locations for large allocations. For example
   `salloc`, `kmalloc` must start at a pre planned virtual address. This is because there is no VMM
   which can search and provide free virtual pages.
2. All physical memory need to be committed from the start. There can be no on-demand allocation of
   physical pages with VMM. Which means that before we call `kpg_map` we must have all physical pages
   allocated.
3. As a consequence of the above condition, we will not only have contiguous virtual pages allocated
   for a module, but physical pages will match.

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
`0xFFC00000 – FFC00FFC`.

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
