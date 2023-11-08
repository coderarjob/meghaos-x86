# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Physical memory map
categories: feature, x86

### Memory Layout after boot0

```
| Range               | Size  | Usage                                                |
|---------------------|-------|------------------------------------------------------|
| 0x000000 - 0x0017FF | 2 KB  | 256 IVT entries                                      |
| 0x001800 - 0x006BFF | 21 KB | Free                                                 |
| 0x006C00 - 0x007BFF | 4 KB  | Stack, size as mentioned in boot0.s                  |
| 0x007c00 - 0x007FFF | 1 KB  | boot0 binary                                         |
| 0x008000 - 0x017FFF | 64 KB | boot1 binary. Max size is 64KB, reusing boot0 buffer |
| 0x018000 - 0x027FFF | 64 KB | boot1 buffer                                         |
|---------------------|-------|------------------------------------------------------|
```

### Memory Layout after boot1

Features/Changes from the last:
1. Free regions at the beginning are removed for the ease of paging.
2. Memory region starts and ends at 4KB page boundaries.

```
| Physical address    | Size   | Usage                                              |             |
|---------------------|--------|----------------------------------------------------|-------------|
| 0x000000 - 0x000FFF | 4 KB   | Boot0 IVT. Kernel IDT (256 IDT entries)(see todo)  | OS Reserved |
| 0x001000 - 0x001FFF | 4 KB   | 512 GDT entries                                    | OS Reserved |
| 0x002000 - 0x002FFF | 4 KB   | Boot Info. 11 file items & 201 memory map items    | OS Reserved |
| 0x003000 - 0x022FFF | 128 KB | User stack. boot0, boot1 space reused. (See Issue) | OS Reserved |
| 0x023000 - 0x042FFF | 128 KB | Kernel Stack                                       | OS Reserved |
| 0x043000 - 0x044FFF | 8 KB   | Kernel PD/PT                                       | OS Reserved |
| 0x045000 - 0x045FFF | 4 KB   | PAB                                                | OS Reserved |
| 0x046000 - 0x07FFFF | 232 KB | Free (End of 1st Free* region: Minimum required)   | Dynamic     |
| 0x080000 - 0x09FBFF | 127 KB | Free (End of 1st Free* region: Maximum on x86)     | Dynamic     |
| 0x100000 - 0x1AFFFF | 704 KB | 11 module files, each of 64KB max size             | OS Reserved |
|---------------------|--------|----------------------------------------------------|-------------|
```

TODO:
`0x0000 - 0x000FFF` physical range should be kept for the Real Mode IVT and not overridden by the
protected mode Kernel, otherwise entering real mode will become difficult and error prone.

Note:
1.  Given that the initial PT/PD, PAB resides from 0x43000 to 0x45FFF, and kernel static allocation
    starts immediately after from 0x46000, one can think that the later starts at 0x43000 because
    the initial tables & PAB are static allocations which are never deallocated.

2.  The maximum boot1 and kernel size is due to limitation of the boot0 FAT routine. It can load
    files of at most 64 KB in size.

3. The 1 KB for Boot Info structure is arbitrarily large. The 1 KB is size should be large for any
   x86_64 system.

**ISSUE**: The same kernel stack was also used in user mode, which caused stack corruption. The CPU
sets the stack pointer to `Kernel Stack Top` when going from user mode to kernel mode.
Then any Push in the Kernel space will overwrite the user stack. The SS & ESP itself will be
restored when returning from an Interrupt, however the because the memory remains the common, it
gets modified by the stack operations of the Kernel mode.

_Solution_: This is temporary solution. I reduced the Kernel stack to make space for the User stack.
Such a separate fixed user stack may not be required as user stack allocation will be part of
process creation - each process will have different physical memory allocated for their stack.

(`*`) 1st free region in the BIOS memory map.

------------------------------------------------------------------------------
## Physical Memory Map
categories: note, x86, obsolete
_24st Sept 2023_

### Previous Memory map

```
| Physical address    | Size   | Usage                                              |
|---------------------|--------|----------------------------------------------------|
| 0x000000 - 0x000FFF | 4 KB   | Free                                               |
| 0x001000 - 0x0017FF | 2 KB   | Free. For 256 IDT entries                          |
| 0x001800 - 0x0027FF | 4 KB   | 512 GDT entries                                    |
| 0x002800 - 0x002BFF | 1 KB   | Boot Info. 11 file items & 47 memory map items     |
| 0x002C00 - 0x023BFF | 132 KB | User stack. boot0, boot1 space reused. (See Issue) |
| 0x023C00 - 0x043BFF | 128 KB | Kernel Stack                                       |
| 0x043C00 - 0x07FFFF | 241 KB | Free (End of 1st Free* region: Minimum required)   |
| 0x080000 - 0x09FBFF | 127 KB | Free (End of 1st Free* region: Maximum on x86)     |
| 0x100000 - 0x1B0000 | 704 KB | 11 module files, each of 64KB max size             |
|---------------------|--------|----------------------------------------------------|
```

------------------------------------------------------------------------------
## Where to keep the GDT?
categories: note, x86, obsolete
_23st October 2020_

Two possible options:
1. At a fixed physical location, say `0x00800`
2. Keep two separate GDTs one in `boot1` and another in `kernel`. Why two?
   Because the kernel stack area overlaps the memory where `boot1` is loaded.

I thought I will go with option 2. Because it decouples `boot1` and `kernel`.
So I thought, but you see they are anyways linked. Let me explain.

Kernel is loaded at physical location 0x28000. So all the offsets and addresses
(.text and .rodata) are assigned addresses 0x28000 and above at the time of
linking (static linking). So while the kernel is running, changing the GDT to a
value is never going to be viable.

kernel is statically given addresses, so effective addresses must be preserved. 

**EA = Base of SReg + Offset**

We want EA and Offset to be the same. So there is no way we can ever going to
change the Base as well. 

So in the end I am going with option 1. We will keep the GDT at a fixed global
location **0x00800** with storage for 512 entries. Thus the memory looks like
below (With just two segments, however). 

### Memory Layout after boot1

```
* 0x00000  - 0x007FF    -   IDT               2   KB (256 IDT entries)
* 0x00800  - 0x017FF    -   GDT               4   KB (512 GDT entries)
* 0x01800  - 0x07BFF    -   Free             25   KB
* 0x07c00  - 0x07FFF    -   boot0             1   KB 
* 0x08000  - 0x17FFF    -   boot1            64   KB (Maximum boot1 size)
*    -     - 0x27FFF    -   kernel stack    128   KB (boo1 space reused)
* 0x28000  -            -   kernel          ---
```

------------------------------------------------------------------------------

## Why separate stack does not work with GCC
categories: note, x86
_21st October 2020_

The below idea of separate segment for stack is not working because the GCC
compiler generates code that do not work correctly when DS =/= SS.

Here is an example:
```
    lea eax, [ebp+12]
    mov eax, DWORD [eax]
```

EAX has value of EBP+12. If you would want to read the from the address stored
at EAX, the correct format would be `mov eax, DWORD SS:[eax]`, because the
address in EAX belongs to the Stack.

But this is not what the compiler generates, and we end up with reading from
the DS, as as DS =/= SS, we are reading from a different location than
intended. No warning or error from compiler either.

So alas! But come to think of it, a separate Stack segment is acutally useless,
as the processor only either can check for the Upper (Expand-Up) or Lower
(Expand-Down) bounds, not both.

So basically we have just 2 segments, but the memory layout will remain the
same.

```
1. A `null` segment descriptor  	- 
2. A `code` segment descriptor.	    -       0   - 4 GB
3. A `data` segment descriptor.	    -       0   - 4 GB
```

### Memory Layout after boot1

```
* 0x00800  - 0x017FF    -   GDT               4   KB (512 GDT entries)
* 0x01800  - 0x07BFF    -   Free             25   KB
* 0x07c00  - 0x07FFF    -   boot0             1   KB 
* 0x08000  - 0x17FFF    -   boot1            64   KB (Maximum boot1 size)
*    -     - 0x27FFF    -   kernel stack    128   KB (boo1 space reused)
* 0x28000  -            -   kernel          ---
```

One more change tough, the GDT that is setup by the `boot1` is local to it. The
kernel must setup and initialize another GDT of its own. The reason for that is
because, setting up of GDT actually should reside with the kernel, and not with
the bootloader. Also in multiprocessor systems there should be one GDT for each
processor. Which is too much to ask of a bootloader. The `boot1` will setup a
rudimentary protected mode environment, for the kernel (Kernel is written in C
and GCC does not work well with real mode), but the final setup rests with the
Kernel.

Paging and Virtual memory is quite important it seems with the available
tools.

------------------------------------------------------------------------------

## Stack overflow protection using GDT
categories: note, x86
_19th Oct 2020_

Currently there is no separate stack segment in the GDT. It just resides at
some offset in the data segment. This presents the problem that in the event of
stack overflow, it will override the data that lies beneath - processor cannot
check because the stack size is never provided to the processor.

```
1. A `null` segment descriptor  	- 
2. A `code` segment descriptor.	    -       0   - 4 GB
3. A `data` segment descriptor.	    -       0   - 4 GB
```

### GDT proposed:
So it is beneficial for the stack to have a overlapping but separate segment in
the GDT. The new GDT should look like this.

```
1. A `null` segment descriptor  	- 
2. A `code` segment descriptor.	    -       0   - 4 GB
3. A `data` segment descriptor.	    -       0   - 4 GB
4. A `stack` segment descriptor.    - 0x08000   - 128 KB
```

### Memory Layout after boot1

```
* 0x00800  - 0x017FF    -   GDT               4   KB (512 GDT entries)
* 0x01800  - 0x07BFF    -   Free             25   KB
* 0x07c00  - 0x07FFF    -   boot0             1   KB 
* 0x08000  - 0x17FFF    -   boot1            64   KB (Maximum boot1 size)
* 0x08000  - 0x27FFF    -   kernel stack    128   KB (boo1 space reused)
* 0x28000  -            -   kernel          ---
```

The boot1 will setup the kernel GDT fully and the GDT will itself be placed at
location 0x800. It will have 4 entries before jumping to the kernel.

<strike>
Note that in order for the processor to check for stack overflow, the stack
segment should be a 'Expand down' segment. However, in that case the processor
will not check for under-flow. __Is there no bound check?__
</strike>

The above is striked because, having just a Expand Up segment is enough for the
processor to check both the upper and lower bounds. 

```
|--------|-----------|----------------------|------------------------|
| Origin |    Limit  |  Valid offset range  |   Valid Address range  |
|--------|-----------|----------------------|------------------------|
| 0x8000 |  0x1FFFF  |  0x0000 - 0x1FFFF    |   0x8000 - 0x27FFF     |
|--------|-----------|----------------------|------------------------|
```

So with a GDT with `Origin = 0x08000` and `Limit = 0x1FFFF`, 
the Valid range of offset is 0x0000 and 0x1FFFF (Limit value). Thus both
overflow and underflow can be checked.

The Kernel can add/override the existing entries, but the GDT itself will
always remain at 0x800. Just like the IDT has a special memory location.
