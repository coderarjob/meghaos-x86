## Megha Operating System V2 - x86
## Virtual Memory Manager
------------------------------------------------------------------------------
__16 Dec 2022__

In order to make changes to a PDE or PTE, we require the virtual addresses of
them. However, the PD addresses in CR3 and PT address in a PDE are all physical.

Now, as there will always be one PD per process we its virtual address in the
entry for the process in the Process table. However, there can be thousands of
page tables per process, storing virtual address of each separately is going to
be expensive - 4KB pre PD.

4KB does not sound too bad right, even with 100 processes, this will account to
just 400KB. So I am sure some OS has/will choose this route.

Having a separate table for the virtual addresses means, that the whole PD is
available and so the full 4 GB of virtual address space can be used.

MMURTL chooses to not have a separate table, but multipurpose the PD for this.
It does this by limiting the virtual address space of each proess to 2GB, and
uses the higher PD entries to store the virtual addresses of pages tables (that
are referenced in the PD).

One problem with storing virtual addresses of page tables separately is that now
this table need to be kept synchronised with the PD.

Recursive mapping is a much better.

### Recursive mapping

Here is how a virtual address to physical address map works - Going from MSB to
LSB, the first 10 bits is index to select a PDE, the next 10 bits is index to
select a PTE, the next 12 bits is used to access each byte in the page.

In recursive mapping, the last PDE has the address of the PD instead of a PT.
With this simple change now, each of the PT in the PD now has a corresponsing
virtual address.

Here is how a virtual address to PT works - Going from MSB to LSB, the first 10
bits is index to select a PDE (here it is always 0x3FF, select the 1024th PDE),
the next 10 bits is index to select a PDE (this is the PT we want to access),
the next 12 bits is the offset into the page table (to access PTEs in the PT).

Special case of this is when the second 10 bits is also 0x3FF, then the 12 bits
becomes of the offset into the PD and can be used to access PDEs - that is
virtual address in the range FFFFF000 – FFFFFFFC access PDEs in the PD.
