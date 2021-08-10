## Megha Operating System V2 - x86
## Paging
------------------------------------------------------------------------------
_18th Dec 2020_

Where should we initiate and start using paging? 
 * In `boot1` or 
 * In `kernel`

I think it should be in `kernel`. This is because of the following reasons.

 1. If at some point I started using GRUB, as the bootloader, the Paging must
    be started from kernel, as GRUB itself does not turn on paging.

 2. If implemented in `boot1`, then the part of the code that comes after, need
    to be properly mapped. 

    **May be `ORG VM_START` is the way to do it**

  3. The `boot1` becomes tied to the details of paging implementation (at-least
     only to be overridden by the `kernel`).

------------------------------------------------------------------------------
_24 July 2021_

How to use page tables to map Virtual Addresses to Physical Addresses?

### Choose a mapping

1. Kernel Physical Address (kpm):
   Location where the kernel is loaded, or could be taken as 0000 address.

2. Kernel Virtual Address (kvm):
   Location you want to use to get to the kernel from user programs. This is the higher virtual 
   address you want for your kernel. Memory __at or above__ `kvm` is available for the kernel, and 
   memory __less__ than `kvm` is available for the user programs.

#### kpm = 0x1000, kvm = 0xC0000000

This is possible, but we will not be able to access memory lower than 0x1000. This is therefore,
not a viable mapping. (There will be no mapping of physical addresses from 0000 to 0x0FFF in 
the page tables)

#### kpm = 0x1000, kvm = 0xC0001000
```
                                                                    0x1000               0x0000  
                                                                      ^                    ^
   Physical memory: --------------------------------------------------|--------------------|

                                                                 0xC0001000          0xC0000000
                                                                      ^                    ^
   Virtual memory:  --------------------------------------------------|--------------------|
```

#### kpm = 0x0000, kvm = 0xC0000000
```
                                                                    0x1000               0x0000  
                                                                      ^                    ^
   Physical memory: --------------------------------------------------|--------------------|

                                                                 0xC0001000          0xC0000000
                                                                      ^                    ^
   Virtual memory:  --------------------------------------------------|--------------------|
```

You can see that the above two cases, the mapping remains the same i.e. Same virtual address maps
to the same physical address. Because mappings are the same, we will have the same paging 
structures and tables.

### Page directories and CR3 register

Choose any 4KB aligned physical address to store the Page directory entries. 
In general depending on the requirement, you may need to allocate storage form 1 to 1024 such 
directory entries.

CR3 [31:12] = Physical Address of the first page directory / 0x1000.

In the above example, we may place the page directory at physical location 0x1000. 
Store the same in CR3.

#### Address translation in paging

```
                 31               22 21               12 11                    0 
Linear Address: |- - - - - - - - - -|- - - - - - - - - -|- - - - - - - - - - - -|
                                                        |<------ 4kB ---------->| 
                                    |<-----------------4 MB-------------------->| 
                |<--------------------------4 GB------------------------------->|
```

1.  **[11:0] Linear address**

Selects byte within a 4 kB page.

2. **[21:12] Linear address**

Selects which page. 
Indexes into the page table to get the start address of the 4kB page.

0xMM gets the address of the (MM+1)th 4 kB page, from the current page table.

 * 0x00     - Gets address of the 1st page.        (from the current page table)
 * 0x3FF    - Gets the address of the 1024th page. (from the current page table)

3. **[31:22] Linear address**

Selects which page table.
Indexes into the page directory table to get the start of the page table.

0xMM gets the address of the (MM+1)th page table, from the page directory table.

 * 0x00     - Gets address of the 1st page table.       (from the page directory table)
 * 0x300    - Gets the address of the 769th page table. (from the page directory table)

#### Considerations for higher half kernel mapping.

Consider we have 
* kvm = 0xC0001000
* kpm = 0x00001000
* We want the map VGA physical memory 0xB8000 to the 1024th page.

What will be the setup?

Before we begin, let us come up with a way to denote a page table.

```
1.  ith page table entry, in jth Page directory entry is denoted by P(jd,i).
    To keep it simple, i, j start from 0. 
    0th page table of 0th page directory -> P(0d,0p).

    P(jd,i) = 0xMM denotes that 0xMM is placed in [31:12] of page table entry.

2. Physical addresses/locations are written in bold.
```

* Placement of the page directory table is arbitory. Lets say we place it at physical location
  **0x1000**.

* We place the 1st page table at location **0x2000** (4 kB bytes from the start of page directory 
  table).

* In out scheme, 0xC0001000 maps to **0x1000**. Which is same as 0xC0000000 mapping to **0x0000**.

* kvm of 0xC0000000 mapping to **0x0000**, means that every virtual address will be equal to or
  higher than 0xC0000000.
  For example: **0x200** of physical location is 0xC0000200 in virtual address.

* As mentioned above [31:22] bits of the logical/virtual address, selects the page directory.
  If 0xC0000000 is our lowest address, then, 0x301st (index = 0x300) page directory entry will be 
  selected first. In other words, every address from 0xC0000000 will index page directories 
  from 768 to 1023.

* P(768d,0)    = 0x00  -> 0x00000 starts the 1st page.
* P(768d,1023) = 0xB8  -> 0xB8000 starts the 1024th page.

### Calculation of logical address given physical address.

In general, application programs, should not worry about physical addresses. If however, such
calculation is required, then, this is mostly done during development and such address is 
hard coded.

1. i = Search page tables, for an page table entry equal to `physical address/0x1000` value. 
       Take the page entry index.
2. m = address of the start of this page table.
3. j = Search page directory table for an directory entry equal to `m/0x1000`. 
       Take the directory index.

Logical address:
[31:22] : j
[21:12] : i
[11:0 ] : any offset.

Mathematically this can be done by: j * 1024 * 4096 + i * 4096 + offset.

Logical address for **0xB8000** = 768 * 1024 * 4096 + 1023 * 4096 = 0xC03FF000.
[31:22] : 768
[21:12] : 1023
[11:0 ] : 0

