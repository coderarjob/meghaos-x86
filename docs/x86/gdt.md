## Megha Operating System V2 - x86
## GDT entries
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

Note that in order for the processor to check for stack overflow, the stack
segment should be a 'Expand down' segment. However, in that case the processor
will not check for under-flow. __Is there no bound check?__

The Kernel can add/override the existing entries, but the GDT itself will
always remain at 0x800. Just like the IDT has a special memory location.


------------------------------------------------------------------------------
