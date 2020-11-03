## Megha Operating System V2 - x86
## GDT entries
------------------------------------------------------------------------------
_23st October 2020_

Where to keep the GDT? Two possible options:

1. At a fixed physical location, say `0x00800`
2. Keep two separate GDTs on in `boot1` and another in `kernel`. Why two?
   Because the stack area overlaps the memory where `boot` is loaded.

I thought I will go with option 2. Because it decouples `boot1` and `kernel`.
So I thought, but you see they are anyways linked. Let me explain.

Kernel is loaded at physical location 0x28000. So all the offsets and addresses
(.text and .rodata) are assigned addresses 0x28000 and above at the time of
linking (static linking). So while the kernel is running, changing the GDT to a
value is never going to be viable.

kernel is staticly given addresses, so effective addresses must be preserved. 

**EA = Base of SReg + Offset**

We want EA and Offset to be the same. So there is no way we can ever going to
change the Base as well. 

So in the end I am going with option 1. We will keep the GDT at a fixed global
location **0x00800** with storage for 512 entries. Thus the memory looks like
below (With just two segments, however). 

### Memory Layout after boot1
* 0x00000  - 0x007FF    -   IDT               2   KB (256 IDT entries)
* 0x00800  - 0x017FF    -   GDT               4   KB (512 GDT entries)
* 0x01800  - 0x07BFF    -   Free             25   KB
* 0x07c00  - 0x07FFF    -   boot0             1   KB 
* 0x08000  - 0x17FFF    -   boot1            64   KB (Maximum boot1 size)
*    -     - 0x27FFF    -   kernel stack    128   KB (boo1 space reused)
* 0x28000  -            -   kernel          ---

------------------------------------------------------------------------------
_21st October 2020_

The below idea of separate segment for stack is not working becuase the GCC
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

1. A `null` segment descriptor  	- 
2. A `code` segment descriptor.	    -       0   - 4 GB
3. A `data` segment descriptor.	    -       0   - 4 GB

### Memory Layout after boot1
* 0x00800  - 0x017FF    -   GDT               4   KB (512 GDT entries)
* 0x01800  - 0x07BFF    -   Free             25   KB
* 0x07c00  - 0x07FFF    -   boot0             1   KB 
* 0x08000  - 0x17FFF    -   boot1            64   KB (Maximum boot1 size)
*    -     - 0x27FFF    -   kernel stack    128   KB (boo1 space reused)
* 0x28000  -            -   kernel          ---

One more change tough, the GDT that is setup by the `boot1` is local to it. The
kernel must setup and initialize another GDT of its own. The reason for that is
becuase, seting up of GDT actually should reside with the kernel, and not with
the bootloader. Also in multiprocessor systems there should be one GDT for each
processor. Which is too much to ask of a bootloader. The `boot1` will setup a
rudimentry protected mode environment, for the kernel (Kernel is written in C
and GCC does not work well with real mode), but the final setup rests with the
Kernel.

Paging and Virutal memory is quite importaint it seems with the available
tools.
------------------------------------------------------------------------------
_19th Oct 2020_

Currently there is no separate stack segment in the GDT. It just resides at
some offset in the data segment. This presents the problem that in the event of
stack overflow, it will override the data that lies beneath - processor cannot
check because the stack size is never provided to the processor.

1. A `null` segment descriptor  	- 
2. A `code` segment descriptor.	    -       0   - 4 GB
3. A `data` segment descriptor.	    -       0   - 4 GB

### GDT proposed:
So it is benifitial for the stack to have a overlapping but sepate segment in
the GDT. The new GDT should look like this.

1. A `null` segment descriptor  	- 
2. A `code` segment descriptor.	    -       0   - 4 GB
3. A `data` segment descriptor.	    -       0   - 4 GB
4. A `stack` segment descriptor.    - 0x08000   - 128 KB

### Memory Layout after boot1
* 0x00800  - 0x017FF    -   GDT               4   KB (512 GDT entries)
* 0x01800  - 0x07BFF    -   Free             25   KB
* 0x07c00  - 0x07FFF    -   boot0             1   KB 
* 0x08000  - 0x17FFF    -   boot1            64   KB (Maximum boot1 size)
* 0x08000  - 0x27FFF    -   kernel stack    128   KB (boo1 space reused)
* 0x28000  -            -   kernel          ---

The boot1 will setup the kernel GDT fully and the GDT will itself be placed at
location 0x800. It will have 4 entries before jumping to the kernel.

<strike>
Note that in order for the processor to check for stack overflow, the stack
segment should be a 'Expand down' segment. However, in that case the processor
will not check for under-flow. __Is there no bound check?__
</strike>

The above is striked because, having jsut a Expand Up segment is enough for the
processor to check both the upper and lower bounds. 

|--------|-----------|----------------------|------------------------|
| Origin |    Limit  |  Valid offset range  |   Valid Address range  |
|--------|-----------|----------------------|------------------------|
| 0x8000 |  0x1FFFF  |  0x0000 - 0x1FFFF    |   0x8000 - 0x27FFF     |
|--------|-----------|----------------------|------------------------|

So with a GDT with `Origin = 0x08000` and `Limit = 0x1FFFF', 
the Valid range of offset is 0x0000 and 0x1FFFF (Limit value). Thus both
overflow and underflow can be checked.

The Kernel can add/override the existing entries, but the GDT itself will
always remain at 0x800. Just like the IDT has a special memory location.
------------------------------------------------------------------------------
