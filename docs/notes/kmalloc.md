## Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Kmalloc working
categories: feature, independent
_17 March 2024_

kmalloc allocates memory from a buffer of pre-allocated memory. It uses this buffer also for keeping
track of allocated and free regions. This document will go into some detail about my implementation.

The buffer can be thought of as having sections. Each of these sections are either used or free. At
the start of each section is the kmalloc header, which keeps track the number of bytes in the
section and whether it is free or used. This header also holds node of various lists.

Headers of multiple sections are linked to one another to form a list, and each header can be part 
to multiple lists.

We have two lists which track free and used regions in the buffer. The nodes of these lists reside
in the kmalloc headers of respective sections. This header also stores the section "net size" and
whether the section is free or used. There is another list called the Adjacent list, which is
described later.

1. Free list      - Links headers of sections which are unallocated.
2. Allocated list - Links headers of sections which are allocated.
3. Adjacent list  - As sections are created and joined, this list contains every section in sequence
   as they exist in kmalloc buffer. This list is used for combining consecutive free sections
   together (to reduce fragmentation).

Sections in the free and allocation list can appear in any order - last added section first or last
added section last. This becomes a problem when we want to merge two or more consecutive freed
sections. This is the reason for the adjacent list. In the adjacent list, the order is important and
sections apprear in the order they exist in the kmalloc buffer.

I am curious what will happen if we only have the adjacent list. It enlists every section so in 
theory kmalloc can operate with this one list. The search operations (for suitable free section and
allocated section) would become lengthy having to traverse the whole list every time. I would like
to know the impact.

The headers store the section "net size". Net size is the usable number of bytes in a section plus
the size of the header. Which means "net size" is the total size (in bytes) of a section. I store
net size and not the "usable" size because I believe that having the total section size would help
in debugging.

This would mean that even if there is a section with zero usable bytes, the section size would still
be non-zero (then would be equal to the size of the header). 

These lists are the bookkeepings which keep track of the used and freed bytes in the kmalloc buffer,
so it is very important that that the whole buffer is accounted for in the lists that is
* Total size of sections in the free list + Total size of sections in the free list = buffer size.
* Total size of every section in the adj list = buffer size.

At the time of allocation, kmalloc first searches for a section whose "net size" >=
"requested net size". If such a section was found, it splits that into two. One half of "requested
net size" bytes is added to the allocation list and the remaining bytes will form a new section that
is added to the free list. To ensure that the later section never goes out of the buffer boundaries,
kmalloc actually searches for a section whose "net size" >= "requested net size" + header size. This
makes sure that there is at least "header size" amount of bytes in the section after the split.
